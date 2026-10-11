## Ninv
An x86_64 assembly project implementing a function, callable from C, to calculate for `n-bit` integer `x` (x > 1)
positive n-bit integer `y` such that: 
```
$x \time y \le 2^n < x(y + 1)$
```

## Project summary
* **Efficiency:** handles integers up to **256,000 bits**.
* **Data representation:** operates on $n$-bit unsigned integers stored in **little-endian** order as arrays of 64-bit words (`uint64_t`).

## Compilation & Usage

```bash
# Assemble solution
nasm -f elf64 -w+all -w+error -o ninv.o ninv.asm

# Compile test 
g++ -c -Wall -Wextra -std=c++20 -O2 -o ninv_example.o ninv_example.cpp

# Link executable
g++ -z noexecstack -o ninv_example ninv_example.o ninv.o -lgmp
```
