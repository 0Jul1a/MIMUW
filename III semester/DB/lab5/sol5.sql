WITH step_e AS (
    SELECT e.tgt e_tgt, count(*) c
    FROM E e
    WHERE e.src = 0
    GROUP BY e.tgt
),
step_f AS (
    SELECT f.tgt f_tgt, sum(e1.c) c
    FROM E f
    JOIN step_e e1 ON e1.e_tgt = f.src
    GROUP BY f.tgt
),
step_g AS (
    SELECT g.tgt g_tgt, sum(f1.c) c
    FROM E g 
    JOIN step_f f1 ON f1.f_tgt = g.src 
    GROUP BY g.tgt
),
step_h AS (
    SELECT h.tgt h_tgt, sum(g1.c) c
    FROM E h
    JOIN step_g g1 ON g1.g_tgt = h.src
    GROUP BY h.tgt
),
step_i AS (
    SELECT i.tgt i_tgt, sum(h1.c) c
    FROM E i
    JOIN step_h h1 ON h1.h_tgt = i.src
    GROUP BY i.tgt
)
SELECT sum(c) count FROM step_i;

