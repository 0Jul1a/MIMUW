package cp2025.engine;

import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.stream.Collectors;

import cp2025.engine.Datalog.*;


public class ParallelDeriver implements AbstractDeriver {
    private final int numWorkers;


    public ParallelDeriver(int numWorkers) {
        this.numWorkers = numWorkers;
    }

    @Override
    public Map<Atom, Boolean> derive(Program input, AbstractOracle oracle)
            throws InterruptedException {

        Map<Predicate, List<Rule>> predicateToRules = input.rules().stream().collect(Collectors.groupingBy(rule -> rule.head().predicate()));
        final ConcurrentMap<Atom, Boolean> knownStatements = new ConcurrentHashMap<>();
        final ConcurrentMap<Atom, ConcurrentLinkedQueue<Thread>> workers = new ConcurrentHashMap<>();
        final AtomicBoolean deriveInterrupted = new AtomicBoolean(false);

        List<Callable<Map.Entry<Atom, Boolean>>> tasks = new ArrayList<>();
        for (Atom query : input.queries()) {
            tasks.add(new ParallelDeriverState(input, predicateToRules, oracle, query, knownStatements, workers, deriveInterrupted));
        }

        ConcurrentMap<Atom, Boolean> result = new ConcurrentHashMap<>();

        try (ExecutorService pool = Executors.newFixedThreadPool(numWorkers)) {
            List<Future<Map.Entry<Atom, Boolean>>> futures = pool.invokeAll(tasks);

            for (Future<Map.Entry<Atom, Boolean>> f : futures) {
                try {
                    Map.Entry<Atom, Boolean> entry = f.get();
                    result.put(entry.getKey(), entry.getValue());
                } catch (ExecutionException e) {
                    Throwable cause = e.getCause();

                    if (cause instanceof InterruptedException) {
                        deriveInterrupted.set(true);
                        interruptAllThreads(workers);
                        throw (InterruptedException) cause;
                    } else {
                        throw new RuntimeException(cause);
                    }
                } catch (InterruptedException i) {
                    deriveInterrupted.set(true);
                    interruptAllThreads(workers);
                    throw i;
                }
            }
        } finally {
            deriveInterrupted.set(true);
            interruptAllThreads(workers);
        }

        return result;
    }

    private static void interruptAllThreads(ConcurrentMap<Atom, ConcurrentLinkedQueue<Thread>> workers) {
        for (ConcurrentLinkedQueue<Thread> threads : workers.values())
            if (threads != null)
                for (Thread t : threads)
                    if (t.isAlive() && t != Thread.currentThread())
                        t.interrupt();
    }

    private static class ParallelDeriverState implements Callable<Map.Entry<Atom, Boolean>> {

        private final Program input;
        private final AbstractOracle oracle;
        private final Atom query;
        private final Set<Atom> inProgressStatements = new HashSet<>();

        private final ConcurrentMap<Atom, Boolean> knownStatements;
        private final ConcurrentMap<Atom, ConcurrentLinkedQueue<Thread>> workers;
        private final AtomicBoolean deriveInterrupted;
        private final Map<Predicate, List<Rule>> predicateToRules;

        public ParallelDeriverState(Program input, Map<Predicate, List<Rule>> predicateToRules, AbstractOracle oracle, Atom query, ConcurrentMap<Atom, Boolean> knownStatements, ConcurrentMap<Atom, ConcurrentLinkedQueue<Thread>> workers, AtomicBoolean deriveInterrupted) {
            this.input = input;
            this.oracle = oracle;
            this.query = query;
            this.predicateToRules = predicateToRules;
            this.knownStatements = knownStatements;
            this.workers = workers;
            this.deriveInterrupted = deriveInterrupted;
        }

        @Override
        public Map.Entry<Atom, Boolean> call() throws InterruptedException {
            try {
                if (knownStatements.containsKey(query))
                    return Map.entry(query, knownStatements.get(query));

                DerivationResult res = deriveStatement(query);
                return Map.entry(query, res.derivable);
            } catch (InterruptedException i) {
                throw new InterruptedException();
            }
        }

        private void addThread(Atom goal) {
            inProgressStatements.add(goal);
            workers.computeIfAbsent(goal, k -> new ConcurrentLinkedQueue<>()).add(Thread.currentThread());
        }

        private void removeThread(Atom goal) {
            inProgressStatements.remove(goal);
            ConcurrentLinkedQueue<Thread> threads = workers.get(goal);
            if (threads != null) {
                threads.remove(Thread.currentThread());
                if (threads.isEmpty()) {
                    workers.remove(goal, threads);
                }
            }
        }

        private void interruptAllThreadsAtom(Atom goal) {
            ConcurrentLinkedQueue<Thread> threads = workers.get(goal);
            if (threads != null && !threads.isEmpty())
                for (Thread t : threads)
                    if (t != null && t != Thread.currentThread())
                        t.interrupt();
        }

        private record DerivationResult(boolean derivable, Set<Atom> failedStatements) {}

        public DerivationResult deriveStatement(Atom goal) throws InterruptedException {

            if (knownStatements.containsKey(goal))
                return new DerivationResult(knownStatements.get(goal), Set.of());


            if (Thread.currentThread().isInterrupted()) {
                if (deriveInterrupted.get())
                    throw new InterruptedException();

                if (knownStatements.containsKey(goal))
                    return new DerivationResult(knownStatements.get(goal), Set.of());
            }

            if (oracle.isCalculatable(goal.predicate())) {
                addThread(goal);
                try {
                    boolean result;
                    try {
                        result = oracle.calculate(goal);
                    } catch (InterruptedException i) {
                        if (deriveInterrupted.get())
                            throw i;
                        else {
                            if (knownStatements.containsKey(goal))
                                return new DerivationResult(knownStatements.get(goal), Set.of());
                            return new DerivationResult(false, Set.of(goal));
                        }
                    }
                    Boolean knownPrev = knownStatements.putIfAbsent(goal, result);
                    if (knownPrev == null) {            // Dodano teraz wynik, trzeba powiadomić wszystkie wątki, które próbują wyprowadzić wyrażenie.
                        interruptAllThreadsAtom(goal);
                    }
                    return new DerivationResult(result, Set.of());
                } finally {
                    removeThread(goal);
                }
            }

            if (inProgressStatements.contains(goal))
                return new DerivationResult(false, Set.of(goal));

            addThread(goal);
            try {
                DerivationResult result = deriveNewStatement(goal);
                removeThread(goal);
                if (result.derivable) {
                    Boolean knownPrev = knownStatements.putIfAbsent(goal, true);
                    if (knownPrev == null) {
                        interruptAllThreadsAtom(goal);
                    }
                } else {
                    if (inProgressStatements.isEmpty()) {
                        for (Atom s : result.failedStatements) {
                            Boolean knownPrev = knownStatements.putIfAbsent(s, false);
                            if (knownPrev == null)
                                interruptAllThreadsAtom(s);
                        }
                    }
                }
                return result;
            } finally {
                removeThread(goal);
            }
        }

        private DerivationResult deriveNewStatement(Atom goal) throws InterruptedException {
            List<Rule> rules = predicateToRules.get(goal.predicate());
            if (rules == null)
                return new DerivationResult(false, Set.of(goal));

            Set<Atom> failedStatements = new HashSet<>();

            for (Rule rule : rules) {
                Optional<List<Atom>> partiallyAssignedBody = Unifier.unify(rule, goal);
                if (partiallyAssignedBody.isEmpty())
                    continue;

                List<Variable> variables = Datalog.getVariables(partiallyAssignedBody.get());
                FunctionGenerator<Variable, Constant> iterator = new FunctionGenerator<>(variables, input.constants());
                for (Map<Variable, Constant> assignment : iterator) {
                    if (Thread.currentThread().isInterrupted()) {
                        if (deriveInterrupted.get())
                            throw new InterruptedException();
                        if (knownStatements.containsKey(goal))
                            return new DerivationResult(knownStatements.get(goal), Set.of());
                    }

                    List<Atom> assignedBody = Unifier.applyAssignment(partiallyAssignedBody.get(),
                            assignment);
                    DerivationResult result = deriveBody(assignedBody);
                    if (result.derivable)
                        return new DerivationResult(true, Set.of());
                    failedStatements.addAll(result.failedStatements);
                }
            }
            failedStatements.add(goal);
            return new DerivationResult(false, failedStatements);
        }

        private DerivationResult deriveBody(List<Atom> body) throws InterruptedException {
            for (Atom statement : body) {
                if (Thread.currentThread().isInterrupted()) {
                    if (deriveInterrupted.get())
                        throw new InterruptedException();

                    if (knownStatements.containsKey(statement))
                        if(!knownStatements.get(statement))
                            return new DerivationResult(false, Set.of());
                        else
                            continue;
                }
                DerivationResult result = deriveStatement(statement);
                if (!result.derivable)
                    return new DerivationResult(false, result.failedStatements);
            }
            return new DerivationResult(true, Set.of());
        }
    }
}