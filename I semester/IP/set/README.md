## SET Card game simulation

- - -

This program simulates the end-phase of the card game SET. It processes a sequence of cards and deterministically
finds and removes the first available set from table according to the rules.


### Key features
* **Deterministic Simulation:** always selects the lexicographically first available set based on card positions on the table.
* **Card Encoding & Decoding:** all cards are encode into unique 4-digits representations in the range `1111` to `3333`.
* **Dynamic Table Management:** maintains table state dynamically according to SET game rules.
* **Language**: C (C17).

- - -  

### Game rules & encoding

The game begins with 12 cards laid face up on the table. In a real-time game, when a player identifies a set,
they reserve the table, claim the set, and 3 new cards are drawn from the deck and add to the table. 
If no player can find a set, 3 additional cards are added onto the table. 
The game ends when the deck is empty and no further sets can be formed.
Each player earns points equivalent to the number of SETs they claimed. 

Each card is represented as a 4-digit number `ABCD` with digits from `{1, 2, 3}` mapping to attributes:

| Digit Position | Attribute | `1` | `2` | `3` |
| :--- | :--- | :--- | :--- | :--- |
| **A** | Number | One | Two | Three |
| **B** | Color | Red | Green | Purple |
| **C** | Filling | Open | Striped | Solid |
| **D** | Shape | Diamond | Squiggle | Oval |

#### Forming a Set
**3** cards form a **SET** if, for every individual attribute, the values are **all identical** or
**all different** across all three cards. 

#### Lexicographical Selection Priority
A set at positions $x < y < z$ takes priority over a set at $x' < y' < z'$ if:
$$(x < x') \quad \text{or} \quad (x = x' \land y < y') \quad \text{or} \quad (x = x' \land y = y' \land z < z')$$

- - -

### Input format
* Sequence of unique 4-digit integers.
* The total number of cards is divisible by 3.
* Numbers can be separated by spaces and line breaks.
* The first 12 cards (or all if $< 12$) are placed on the table; remaining cards form the deck.

### Output format
* The initial table state.
* Sequence of moves until the game ends.
* After each move, the current state of the table.
* The output traces the simulation step-by-step:
  *`=` – prints the current table state preceded by an `=` character.
  *`-` – indicates a removed SET (followed by the 3 cards in table order).
  *`+` – indicates 3 additional cards drawn from the deck when no SET is available on the table.


### Formal grammar
```
Result            -> Table MoveSequence
MoveSequence      -> ε | Move Table MoveSequence
Table             -> '=' CardSequence '\n'
CardSequence      -> ε | ' ' Card CardSequence
Card              -> Digit Digit Digit Digit
Digit             -> '1' | '2' | '3'
Move              -> Set | Add
Set               -> '-' CardSequence '\n'
Add             -> '+' '\n'
```

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
gcc @opcje setd.c -o set
```

#### Memory leak testing (valgrind)ą
``` bash
valgrind --leak-check=full -q --error-exitcode=1 ./set
```
