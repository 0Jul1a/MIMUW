## Sokoban game

This program simulates a Sokoban game where the player's goal is to move boxes to their target destinations
on the board without getting stuck. 
It uses DFS logic to validate and discover reachable paths for player positioning, while providing movement history and unlimited undo functionality.

- - - 

### Key features
* **Automatic player pathfinding:** the player specifies only the box to move and the direction, the program automatically validates
  whether this move is legal and calculated a valid path to required position using DFS logic.
* **Undo movements:** an unlimited moves history allowing player to undo previous box pushes and restore previous board states.
* **Named boxes:** all boxes have unique names (`a`-`z`) and their target goals (`A`-`Z`).
* **Dynamic board:** supports custom board state parsed from input.

- - -

### Board symbol representation

| Symbol | Description |
| :---: | :--- |
| `-` | Empty cell (not a target) |
| `+` | Empty cell  (target) |
| `#` | Wall |
| `@` | Player on an empty cell |
| `*` | Player on a target cell |
| `a` .. `z` | Box on an empty cell |
| `A` .. `Z` | Box on a target cell |

Each board contains only **one** player (`@/*`), each box letter appears at most once.

- - -

### Input format
The input consist of an initial board state description, followed by a non-empty sequence of commands, ending with a dot (`.`).
- - -

### Commands
Commands are executed sequentially until terminated by a dot (`.`), any data after that is ignored.

* **Print state: (`\n`)** print the current board state.
* **Push box: (`[box][direction]`)**
  * box: lowercase letter `a`-`z`
  * directions:
    * `8`: Up
    * `2`: Down
    * `4`: Left
    * `6`: Right
Example: `a6` pushes box `a` to the right if move is reachable.
* **Undo move: (`0`)** undo the last valid box push and returns the player to last place. If there was no valid push last time, the commend is ignored.

- - -

### Output format
* Board states printed in response to commands, each has the same format as the input board description.

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
gcc @opcje sokoban.c -o sokoban
```

#### Memory leak testing (valgrind)
``` bash
valgrind --leak-check=full -q --error-exitcode=1 ./sokoban
```
