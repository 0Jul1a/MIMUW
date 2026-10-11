## Internet radio client

A C++ implementation of an internet radio client communicating with a remote server over TCP, supporting IPv$ and IPv6.

- - - 

### Key features 
* **Dual-Stack network engine (IPv4 & IPv6):** Resolves stream URLs using `getaddrinfo`, automatically picking protocol versions or enforcing explicit IPv4 (`-4`) or IPv6 (`-6`) socket bindings.
* **Protocol & metadata multiplexing:** Optional ICY metadata negotiation (`-m`). When enabled, audio data and inline text metadata chunks are demultiplexed in real time. Audio data is streamed cleanly to `stdout` without frame corruption or stutter, while ICY title updates are routed to `stderr`.
* **Interactive terminal control:** Graceful shutdown on user command (`quit`) or EOF, flushing all buffered stream data before terminating with exit status `0`.
* **TLS/SSL encryption support:** fully supports secure https streams using OpenSSL (`libssl` / `libcrypto`).

- - -

## Compilation

```bash

make all

# Clean build artifacts
make clean
```

- - -

### Command Line Options

```bash
./sikradio -u <url> [-m] [-t <timeout_ms>] [-4 | -6] [-v <0-4> | -q]

-u url : the url of the radio stream (required)
-m m   : enable ICY metadata multiplexing
-t     : network timeout in ms
-4 / -6: force IPv4 or IPv6
-v     : set verbosity level (0-4) for internal debugging
-q     : v = v0
