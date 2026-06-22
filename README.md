<div align="center">

# 🔐 Decipher

**A multithreaded XOR block cipher with a TCP client/server, written in pure C.**
The client encrypts a file block by block across several threads and ships it
over the network; the server decrypts it in parallel and saves the recovered text.

![Language](https://img.shields.io/badge/Language-C-blue?style=flat-square)
![Build](https://img.shields.io/badge/Build-Make-orange?style=flat-square)
![Dependencies](https://img.shields.io/badge/Dependencies-pthread-4c1?style=flat-square)
![License](https://img.shields.io/badge/License-MIT-lightgrey?style=flat-square)


</div>

---

## 🚀 What it does

Two cooperating programs move a file across the network while keeping it
ciphered in transit:

- **🔐 XOR block cipher** — the text is padded and split into 8-byte blocks (`LEN_BLOCK`), and every block is XOR-ed with an 8-byte key. Because `x ^ k ^ k == x`, decryption is the very same operation (`modify_blocks`).
- **🧵 Parallel processing** — both the client (encryption) and the server (decryption) spread the blocks as evenly as possible across `p` worker threads.
- **🌐 TCP transport** — a length-prefixed message protocol carries the ciphertext, its lengths and the key; an `ACK` closes the handshake.
- **🧑‍🤝‍🧑 Concurrent server** — each accepted client is handled in its own detached thread, up to a configurable number of simultaneous connections.
- **📝 Timestamped output** — the server writes the decrypted text to `prefix + client-ip + timestamp + .txt`.
- **🛡️ Signal masking** — the critical sections are protected by blocking `SIGINT`, `SIGALRM`, `SIGUSR1`, `SIGUSR2` and `SIGTERM`.

> 🗣️ The decrypted text is written to the **output file** on the server, while the
> progress logs of both programs are printed to **stdout/stderr** — so a run can
> be followed live without touching the produced file.

---

## 🗂️ Project structure

```
.
├── README.md
├── assignment.md		   # the project assignment
├── LICENSE
├── Makefile               # build, run and clean targets
├── include/
│   ├── common.h           # shared helpers (errors, signals, blocks, sockets)
│   ├── client.h           # client-only declarations
│   └── server.h           # server-only declarations
└── src/
    ├── common.c           # code shared by client and server
    ├── client.c           # client entry point (encrypt + send)
    └── server.c           # server entry point (receive + decrypt)
```

`common.{c,h}` holds everything the two programs share: error/signal helpers,
local-IP discovery, the block-cipher engine, and the `send_arg` / `receive`
socket wrappers.

---

## ⚙️ Building

| Command | Action |
| :------ | :----- |
| `make` | 🔨 Builds both `bin/client` and `bin/server` |
| `make client` | 🔨 Builds only the client |
| `make server` | 🔨 Builds only the server |
| `make run-server ARGS="..."` | ▶️ Builds and runs the server |
| `make run-client ARGS="..."` | ▶️ Builds and runs the client |
| `make clean` | 🧹 Removes `build/` and `bin/` |
| `make help` | 📖 Lists every target |

Alternatively, manual compilation:

```sh
gcc -Wall -Iinclude -pthread src/common.c src/client.c -o client
gcc -Wall -Iinclude -pthread src/common.c src/server.c -o server
```

You can override the compiler or flags, e.g. to build with the sanitizers:

```sh
make CC=clang
make CFLAGS="-Wall -Wextra -g -std=gnu11 -pthread -fsanitize=address"
```

---

## 📖 Usage

Start the **server** first, then run one or more **clients** pointed at the
server's IP address. The server always listens on TCP port **`49153`** and
prints the address it bound to on startup.

### Server

```
server <threads-per-client> <output-prefix> <max-parallel-clients>
```

| Argument | Description |
| :------- | :---------- |
| `threads-per-client` | Decryption worker threads spawned per client. **> 0.** |
| `output-prefix` | Prefix prepended to every output file name. **≤ 255 chars.** |
| `max-parallel-clients` | Maximum number of clients served at once. **> 0.** |

### Client

```
client <filename> <key> <threads> <server-IP> <server-port>
```

| Argument | Description |
| :------- | :---------- |
| `filename` | Path to the input text file to encrypt. **Name ≤ 255 chars.** |
| `key` | Encryption key — **exactly 8 characters** (one 8-byte block). |
| `threads` | Encryption worker threads. **> 0.** |
| `server-IP` | IPv4 address of the server. **≤ 15 chars.** |
| `server-port` | Server port — use `49153` to match the server. |

> ⚠️ **A note on the key.** It must be **exactly 8 characters** long: it is the
> raw 8-byte block XOR-ed against every block of text. The client and server
> exchange the payload as raw host-endian bytes, so both are expected to run on
> machines with the same architecture/endianness.

---

## 🧪 Example

In one terminal, start the server — 4 threads per client, output prefix `out_`,
up to 8 clients at once:

```sh
make
./bin/server 4 out_ 8
# ... Server IP: 192.168.1.20
# ... Port the server listens on: 49153
```

In another terminal, encrypt and send a file with the 8-character key
`12345678`, using 4 client-side threads, to the server shown above:

```sh
echo "Hello, Decipher!" > message.txt
./bin/client message.txt 12345678 4 192.168.1.20 49153
```

The server reconstructs the original text and saves it to a file such as
`out_192.168.1.20:2026-06-20;17:30:00.txt` containing `Hello, Decipher!`.

---

## 🔬 How it works

```
        ┌──────────────────────────┐                 ┌──────────────────────────┐
        │          CLIENT          │                 │          SERVER          │
        ├──────────────────────────┤                 ├──────────────────────────┤
        │ 1. read input file       │                 │ 1. listen on TCP :49153  │
        │ 2. split into 8-byte     │   TCP segments  │ 2. accept() each client  │
        │    blocks                │ ──────────────▶ │    in its own thread     │
        │ 3. XOR each block with   │  len, C, L, K   │ 3. split ciphertext into │
        │    the key (p threads)   │                 │    blocks                │
        │ 4. send ciphertext + key │                 │ 4. XOR-decrypt (p        │
        │ 5. wait for ACK          │ ◀────────────── │    threads) with the key │
        │                          │      ACK        │ 5. write plaintext file  │
        └──────────────────────────┘                 │ 6. send ACK              │
                                                      └──────────────────────────┘
```

After the TCP connection is established, the client sends four messages in
order (each via the length-prefixed `send_arg` helper), then half-closes the
write side and waits for the acknowledgement:

| # | Field | Type | Description |
| - | :---- | :--- | :---------- |
| 1 | `c_len` | `size_t` | Length in bytes of the ciphertext. |
| 2 | `C` | `c_len` bytes | The encrypted, block-padded payload. |
| 3 | `L` | `size_t` | Length of the original plaintext. |
| 4 | `K` | `unsigned long long` | The 8-byte XOR key. |

The server replies with a single `int` `ACK` (`42`) once the file has been
decrypted and written, after which both sides close the connection.

---

## 🧱 Architecture

The main data structures are defined in [`common.h`](include/common.h),
[`client.h`](include/client.h) and [`server.h`](include/server.h):

| Struct | Role |
| :----- | :--- |
| `C_attr` | Client command-line attributes (file, key, threads, server IP/port). |
| `S_attr` | Server command-line attributes (threads, output prefix, max clients). |
| `Blocks` | The list of fixed-size blocks and their count. |
| `T_data` | Arguments of an encryption/decryption thread (block range + key). |
| `T_c_data` | Arguments of a per-client server thread (socket fd, threads, prefix, IP). |

The functions are grouped by responsibility (error handling, signals, local-IP
discovery, file I/O, the block-cipher engine, socket send/receive) and each one
is documented in Doxygen style in the headers.

---

## ⚠️ Notes & limitations

- **Educational XOR cipher — _not_ secure.** A single-key XOR cipher is trivially breakable and the key travels over the wire in cleartext. Do not use it to protect anything that matters.
- **Endianness-dependent:** integers are sent in raw host byte order, so client and server should share the same architecture.
- **Fixed port:** the server's listening port (`49153`) is set at compile time in [`src/server.c`](src/server.c) (`PORT_TCP`).
- **Linux-oriented:** `get_ip()` selects the active interface using the Linux interface-flag value `4163` (`UP | BROADCAST | RUNNING | MULTICAST`); the flag bits differ on other platforms.

---

## 👥 Authors

Decipher was designed and built by:

- **[LM-official](https://github.com/LM-official)**
- **[Pierba](https://github.com/Pierba)**
- **[ItsSpicci](https://github.com/loremicci)**

---

## 📄 License

Released under the MIT License. See [LICENSE](LICENSE).