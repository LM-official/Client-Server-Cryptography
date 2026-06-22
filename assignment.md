# Assignment — Client-Server Encrypted Communication

## Project Description

Develop a client-server application that allows a client process to send encrypted data to a server process. The server must decrypt the received data and save it to a file. The client and server processes must communicate via **sockets**.

---

## Client Process Specification

### Input

The client application receives as input:

| Parameter | Description |
|-----------|-------------|
| Filename | A text or binary file (a text file is recommended for testing) |
| Key `K` | A 64-bit encryption key (representable as an `unsigned long long int`) |
| Integer `p` | The desired degree of parallelism for the encryption computation |
| Server IP & Port | The IP address and port number on which the server is listening |

### How It Works

1. The client reads the input file and extracts the text `F`, determining its length `L`.
2. It divides `F` into `n` blocks `Bi` of **64 bits** each:

```
F = B1 B2 … Bn
```

3. The last block `Bn` may contain fewer than 64 bits — in that case it must be **padded** with chosen characters (e.g. `'\0'`).
4. For each block `Bi`, the client computes the **bitwise XOR** with key `K`, obtaining the encrypted block:

```
Ci = Bi XOR K
```

5. The encrypted text `C` is the concatenation of all encrypted blocks in order: `C1 C2 … Cn`.
6. The client sends to the server a message containing:

```
[ C1 C2 … Cn , L , K ]
```

7. The client **waits for an acknowledgement** from the server before terminating.

### Client Requirements

- The client must use **at most `p` threads** to perform encryption operations in parallel.
- The client must correctly handle server-side connection closures.
- During encryption and transmission, the client process must **not be interrupted** by the following signals: `SIGINT`, `SIGALRM`, `SIGUSR1`, `SIGUSR2`, `SIGTERM`.
- Any shared data structures used between threads must be designed to **avoid race conditions**.

---

## Server Process Specification

### Input

The server application receives as input:

| Parameter | Description |
|-----------|-------------|
| Integer `p` | The desired degree of parallelism for decryption |
| String `s` | A prefix for the output filenames |
| Integer `l` | The number of connections that can be concurrently handled by the server |

### How It Works

1. The server listens on a specific port and **accepts connection requests** from multiple client processes.
2. It receives a message containing the encrypted text `C`, the original text length `L`, and the 64-bit encryption key `K`.
3. It divides `C` into `n` blocks of 64 bits and — using `p` threads (`p <= n`) — decrypts each block:

```
Bi = Ci XOR K
```

4. Threads store the decrypted blocks in a **shared buffer**.
5. Once fully decrypted, the text is reconstructed by concatenating all `Bi` blocks and saved to a file whose name begins with prefix `s`.
6. **Before writing to the file**, the server sends the **acknowledgement** to the client and closes the connection.

### Server Requirements

- A server thread performing decryption must **not be interrupted** by: `SIGINT`, `SIGALRM`, `SIGUSR1`, `SIGUSR2`, `SIGTERM`.
- A server thread writing decrypted text to disk must **not be interrupted** by: `SIGINT`, `SIGALRM`, `SIGUSR1`, `SIGUSR2`, `SIGTERM`.
- The shared buffer for storing decrypted text must be:
  - **Dynamically allocated**
  - Managed as a **linked list of blocks**
  - Protected against **race conditions**

---

## General Requirements

- Parameters for both the client and server must be passed either as **command-line arguments** or as **environment variable values**.
- Both applications must correctly handle:
  - Errors in input parameters
  - All possible errors from function calls and system calls

---

## Encryption Example

```
F = "il mio nome e' legenda"
```

**Binary representation of F:**
```
01001001 01101100 00100000 01101101 01101001 01101111
00100000 01101110 01101111 01101101 01100101 00100000
01100101 00100111 00100000 01101100 01100101 01100111
01100101 01101110 01100100 01100001
```

**L = 22 bytes = 172 bits**

```
K = "emiliano"
```

**Binary representation of K:**
```
01100101 01101101 01101001 01101100 01101001 01100001 01101110 01101111
```

**n = 3 blocks** (the third block needs 16 bits of zero-padding):

```
B1 = 01001001 01101100 00100000 01101101 01101001 01101111 00100000 01101110
B2 = 01101111 01101101 01100101 00100000 01100101 00100111 00100000 01101100
B3 = 01100101 01100111 01100101 01101110 01100100 01100001 00000000 00000000
```

**Computing C1 = B1 XOR K:**
```
  01001001 01101100 00100000 01101101 01101001 01101111 00100000 01101110
  XOR
  01100101 01101101 01101001 01101100 01101001 01100001 01101110 01101111
  =
  00101100 00000001 01001001 00000001 00000000 00001110 01001110 00000001
```

---

## Testing

Design a series of test cases to verify the specifications and requirements of both the client and server processes. Tests should cover:

- Normal encryption/decryption of a text file
- Files whose size is not a multiple of 64 bits (padding verification)
- Multiple concurrent client connections to the server
- Signal handling during encryption, transmission, and file writing
- Race condition scenarios with shared buffers
- Error cases: invalid parameters, unreachable server, corrupted files
- Verification that the decrypted output matches the original input file exactly
