## Introduction to programming

A collection of **3** core assignments completed during the first semester of the Computer Science program @ MIMUW. 

- - -

### Repository structure

| Project | Folder | Language | Description |
| :--- | :--- | :--- | :--- |
| **Rubik's cube simulator** | [`/rubik_cube`](./rubik_cube) |<img src="https://skillicons.dev/icons?i=c" height="40"> | $N \times N \times N$ Rubik's Cube simulator processing layer rotation streams. |
| **Sokoban game** | [`/sokoban`](./sokoban) |<img src="https://skillicons.dev/icons?i=c" height="40">| Sokoban game simulation featuring DFS pathfinding and full undo functionality. |
| **SET card game simulator** | [`/set`](./set) | <img src="https://skillicons.dev/icons?i=c" height="40">| Deterministic card game simulator implementing lexicographical move selection. |

---

### Requirements & environment

All projects are written in ISO C17 and designed to compile under strict GCC flags without any warnings or memory leaks.

- **Compiler:** `gcc` (version supporting `-std=c17`)
- **Memory leakage analysis:** `valgrind`

> **Note:** Only the implementation files (`.c`, `.asm`) are my own work. All assignment specifications and mathematical formalisms belong to their original authors @ **MIMUW**.
