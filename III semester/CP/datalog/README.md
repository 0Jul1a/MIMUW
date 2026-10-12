## Parallel Datalog deriver 

A concurrent, multi-threaded evaluation engine for an extended Datalog engine.

> **Note** My code is located in `datalog/solution/src/main/engine/ParallelDeriver.java`.
All other files were provided by the course coordinator as a base code and documentation.

- - -

### My contribution

A critical part of this project involved thoroughly reading, analyzing and understanding the provided codebase,
mathematical semantic and specification before implementing the parallel deriver.

**Key features**:
* **Parallel execution:** a multi-threaded evaluation engine that processes multiply Datalog queries concurrently using a fixed-size thread pool.
* **Shared knowledge base:** implemented thread-safe memoization to share derived and non-derivable statements between threads, avoiding redundant computations.
* **Inter-thread communication:** designed an interruption mechanism to instantly notify working threads if another worker resolves a statement they are currently evaluating.
* **Cancellation & cleanup:** handled cancellation of long-running oracle calls and ensured worker threads terminate cleanly, without memory leaks.

- - -

### Repository structure

* `src/main/java/cp2025/datalog/`: automatically generated parser code.
  * Can be regenerated based on the grammar specified in `/Datalog.cf` using the `make datalog` command.
* `src/main/java/cp2025/engine/`:
  * `Datalog.java`: fundamental structures describing a Datalog program (atoms, rules, etc.) with helper functions:
    * `toString`
    * `Datalog.Program.validate()`
    * `List<Variable> getVariables(List<Atom>)`
  * `Parser.java`: wrapper for the generated parser providing static methods that accept program code (as a file path, an open `Reader` character stream, or directly as a `String`) and return a validated `Datalog.Program` structure.
  * `AbstractDeriver.java` & `AbstractOracle.java`: engine and oracle interfaces.
  * `NullOracle.java`: trivial oracle implementation (where no predicate is calculatable).
  * `Main.java`: usage example.
  * `SimpleDeriver.java`: single-threaded engine implementation described above.
  * `ParallelDeriver.java`: **Multi-threaded engine implementation**.
  * `FunctionGenerator.java`: helper class allowing iteration over all assignments.
  * `Unifier.java`: static helper methods for unifying a rule head with a target goal (statement being derived).
* `test/java/cp2025/`: sample basic tests (passing them does not guarantee any points).
  
Additionally, the archive contains:
* `examples/`: examples of Datalog programs with expected results (using `NullOracle`).
* `lib/`: dependencies.
* `.vscode/java-formatter.xml`: automatic code formatting configuration (`Ctrl+Shift+I` in VS Code).
* `Makefile`: makes it easier to use Maven from the console.
* `mvnw`, `mvnw.cmd`: Maven Wrapper executables that allow building the project on any system (working in the `students` environment is still recommended).
* `pom.xml`: Maven project specification (Java version, dependencies).
* `bnfc`: BNFC executable for generating parsers.

- - -

### Compilation & run

``` bash
cd solution
make test
```


