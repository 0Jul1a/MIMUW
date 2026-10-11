## Object oriented Programming - Matrix Slices

### Project description
A Java library implementing 0D (scalars), 1D (vectors) and 2D (matrices) arrays of double values.
It supports polymorphic arithmetic, broad broadcast operations, slicing views, deep copying, 
and strict checked exception handling without external collections.

- - -

## Project overview

The core objective is to model multi-dimensional numeric data structures using fundamental Object-Oriented Design principles in Java (polymorphism, method overriding, and method overloading) while restricted solely to native Java arrays (`double[]`, `double[][]`).

### Supported Array Types
- **Scalar (0D):** Single `double` value ($0$ dimensions).
- **Vector (1D):** $1 \times N$ (horizontal) or $N \times 1$ (vertical) dense array.
- **Matrix (2D):** Rectangular $M \times N$ dense grid of `double` values.

> **Note:** Only the implementation files (`.c`, `.asm`) are my own work. All assignment specifications and mathematical formalisms belong to their original authors @ **MIMUW**.
