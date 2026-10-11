## Databases

A collection of laboratory assignments completed during my third year Databases course @ MIMUW.
The projects cover data modeling, complex analytical queries, recursive graph processing, and query
performance optimization on large datasets.

All scripts are compatible with **Oracle SQL**. 

### Repository structure
| Assignment | Folder | Description |
| :--- | :--- | :--- |
| **Lab 1: Data import & simple 1uerying** | [`/lab1`](./lab1) | Single-table schema creation, CSV data insertion, and conditional data filtering with aggregation. |
| **Lab 2: Relational schema & joins** | [`/lab2`](./lab2) | Normalization into 3 related tables (`Autor`, `Praca`, `Autorstwo`), co-authorship relation extraction, and complex filtering. |
| **Lab 3: Analytical windowing & aggregations** | [`/lab3`](./lab3) | Calculating top-N author publication scores based on proportional point distribution across co-authors. |
| **Lab 4: Graph traversals & Erdős number** | [`/lab4`](./lab4) | Calculating shortest co-authorship graph distances (Pilipczuk Number) using recursive CTEs (`WITH RECURSIVE`). |
| **Lab 5: SQL Query optimization** | [`/lab5`](./lab5) | Rewriting a multi-way self-join graph query on a 100k-row table to reduce execution time from tens of seconds to < 1s. |
