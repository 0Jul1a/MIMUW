## Moore automata

### Project description
A dynamically loaded C library (libma.so) that simulates Moore automata. Supports creating, connecting and simulating
networks of interacting Moore automata.

- - - 

## Project summary
* **Automata description:** `n` one-bit inputs, `m` one-bit outputs, `s-bit` state. Custom transition and output function. 
* **Synchronous Execution:** `ma_step` executes updates across all machines in the network simultaneously. 
* **Memory safety:** strict allocations error handling (`errno` setting to `EINVAL/ENOMEM`) with weak exception safety
  guarantees no leaks under `valgrind`.
* **Connectivity:** supports complex connections between machines: connecting outputs of one machine to inputs of another (`ma_connecyt`).

- - -

## Functions
* `ma_create_full`: creates a custom automaton with user-defined transition and output functions.
* `ma_create_simple`: creates a simplified automaton where number of outputs = number of states and with identity as a output function.
* `ma_delete`: deletes an automaton and frees its memory.
* `ma_connect`: connects outputs of one automaton to inputs of another.
* `ma_disconnect`: disconnects nums of inputs of an automaton.
* `ma_step`: performs one step for a set of automata (all run in parallel and synchronously).
* `ma_set_input`: sets input signals for unconnected inputs.
* `ma_get_output`: returns a pointer to the array of bits representing the current output.
* `ma_set_state`: sets the state of automaton.

- - -

## Compilation & Usage

```bash
# Build dynamic library libma.so
make libma.so

# Clean build artifacts
make clean
```
