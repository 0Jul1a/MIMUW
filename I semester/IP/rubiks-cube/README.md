## Rubik's Cube Simulation
- - -

This program simulates an $N \times N \times N$ Rubik's Cube. It initializes a solved Rubik's cube, processes a 
sequence of layer rotation commands from standard input, and prints the visual representation of the cube's state on demand. 

### Key features:
* **Customizable size:** defined as a symbolic constant at compile time (`-DN=value`); defaults to `5`.
* **Initial state:** starts with a fully solved cube.
* **Move execution:** processes and applies a sequence of commands to rotate specific layers of the cube.
* **State display:** prints the current state of the cube on demand.
* **Language:** C (C17)

- - -

### Input format

The input consist of a sequence of commands terminated by a dot (`.`), any data following the closing dot is ignored.

* **Print command:** a newline (`\n`) requests the program to print the current state of the cube.
* **Rotation commands:** format is `[number of layers][face of the cube][rotation angle]`:
  * **Number of layers:** optional positive integer $i$, ($1 \le i \le N$), if empty then set to `1`.
  * **Face of the cube:** required single character:
    * `u` - Up (Top)
    * `l` - Left
    * `f` - Front
    * `r` - Right
    * `b` - Back
    * `d` - Down (Bottom)
  * **Rotation angle:** optional:
    * `empty`: 90° clockwise
    * `'`: -90° counter-clockwise
    * `"`: 180°

Executing a rotation command with **W layers**, **S face** and **angle K**, rotates **W layers** simultaneously
(viewed from the perspective of face $S$) by **angle K** clockwise.

- - -

### Formal grammar 

```
Data → CommandSequence Period
CommandSequence → ε | Command CommandSequence
Command → Rotation | Print
Rotation → LayerCount Face Angle
Print → NewLine
Face → Letter
LayerCount → One | Many
One → ε
Many → Number
Number → Positive | Number Digit
Angle → Quarter | MinusQuarter | Half
Quarter → ε
MinusQuarter → Apostrophe
Half → Quote
Letter → 'u' | 'l' | 'f' | 'r' | 'b' | 'd'
Positive → '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'
Digit → '0' | Positive
Apostrophe → '\''
Quote → '"'
NewLine → '\n'
Period → '.'
```

- - -

### Output format
The program output is the result of executing all print commands, showing the current visual representation of the cube's layers.
It consists of 3N + 1 lines:
* the first line is empty,
* the next N lines describe the Up face,
* the next N lines describe Left | Front | Right | Back faces,
* the last N lines describe Down face

```
    u
l | f | r | b
    d
```
Each face has its own color:
| Face | Color |
|:--|:--|
| Up | 0 |
| Left | 1 |
| Front | 2 |
| Right | 3 |
| Back | 4 |
| Down | 5 |

- - -

### Compilation & Run

#### Compilation options: 
```bash
-std=c17
-pedantic
-Wall
-Wextra
-Wformat-security
-Wduplicated-cond
-Wfloat-equal
-Wshadow
-Wconversion
-Wjump-misses-init
-Wlogical-not-parentheses
-Wnull-dereference
-Wvla
-Werror
-fstack-protector-strong
-fsanitize=undefined
-fno-sanitize-recover
-g
-fno-omit-frame-pointer
-O1
```
#### Compilation
``` bash
gcc @opcje -DN=wartość rubik.c -o rubik
```

#### Memory leak testing (valgrind)
``` bash
valgrind --leak-check=full -q --error-exitcode=1 ./rubik
```





