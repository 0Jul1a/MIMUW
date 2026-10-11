## Kayles network game

A network-based implementation of the game **Kayles** using a custom application-layer protocol built on top of **UDP (IPv4)** in C++.

This project consist of:
* a **server** that manages the game state,
* a **client** that is a bridge between the player and the server, launched by the player

- - -

### Key features 
* **UDP protocol implementation:** Binary network-protocol processing with network byte order (big-endian) serialization, state synchronization, timeout tracking, and invalid packet diagnostic responses (`MSG_WRONG_MSG`).
* **Bitpacked game board:** Efficient bitmap encoding for $N$ pins ($1 \le N \le 256$) packed into $\lfloor N / 8 \rfloor + 1$ bytes.
* **Error handling & validation:** Strict CLI argument parsing, input sanitization, dynamic ID assignment, and validation of move legality.

- - -

### Application protocol specification
All multi-byte values are transmitted in **network byte order** (big-endian).

- - -

### Game State Layout (`MSG_GAME_STATE` - Type 5)
| Field | Type | Description |
| :--- | :---: | :--- |
| `game_id` | `uint32_t` | Unique game session identifier ($0 \le \text{ID} \le 2^{32}-1$). |
| `player_a_id` | `uint32_t` | Player A ID (non-zero). |
| `player_b_id` | `uint32_t` | Player B ID ($0$ if waiting for opponent). |
| `status` | `uint8_t` | `0` (Waiting A), `1` (Turn A), `2` (Turn B), `3` (Win A), `4` (Win B). |
| `max_pawn` | `uint8_t` | Index of the highest pin position ($0 \le \text{max\_pawn} \le 255$). |
| `pawn_row` | `uint8_t[]` | Bit packed array representing pawn presence (1 = pawn, 0 = no pawn). |

- - -

### Client Message Types
- **`MSG_JOIN` (Type 0):** Request to join or host a new game (`msg_type`, `player_id`).
- **`MSG_MOVE_1` (Type 1):** Knock down 1 pawn at position `pawn`.
- **`MSG_MOVE_2` (Type 2):** Knock down 2 adjacent pawn at positions `pawn` and `pawn + 1`.
- **`MSG_KEEP_ALIVE` (Type 3):** extension of player activity.
- **`MSG_GIVE_UP` (Type 4):** player give up the game.

- - -

### Compilation & build
```bash
# Build both executable targets: kayles_server and kayles_client
make all

# Clean build artifacts
make clean

# Example run for server and client

./server -r 11101111011111 -a 0.0.0.0 -p 2024 -t 30

./client -a 127.0.0.1 -p 2024 -m 0/1 -t 5

