# P2P 

Simple p2p system with each node:
- listens for peers
- connects to peers
- forwards messages

---

## Network Topology
The following diagram illustrates how the main() function initializes the core threads and how those threads subsequently spawn peer handlers 

![Thread Hierarchy and Data Flow](assets/p2p.drawio.svg)

## Project Structure
```
p2p-mini/
│
├── src/
│   │
│   ├── main.c
│   │
│   ├── peer/
│   │   ├── peer.c
│   │   └── peer.h
│   │
│   ├── network/
│   │   ├── network.c
│   │   └── network.h
│   │
│   ├── protocol/
│   │   ├── protocol.c
│   │   └── protocol.h
│   │
│   ├── utils/
│   │   ├── utils.c
│   │   └── utils.h
│   │
│   └── config.h
│
├── tests/
│   ├── test_known_hosts.sh
│   ├── test_broadcast.sh
│   ├── test_loop.sh
│   └── test_framing.sh
│
├── assets/
│
├── build/
│
├── Makefile
│
├── README.md
│
└── .gitignore
```

## How to test

### Compile 

```bash
gcc -pthread \
src/main.c \
src/peer/peer.c \
src/network/network.c \
src/protocol/protocol.c \
-o build/node
```

### Terminal 1

```bash
./build/node 5000
```
### Terminal 2
```bash
./build/node 5001 127.0.0.1 5000
```

### Terminal 3

```bash
./build/node 5002 127.0.0.1 5000
```

type something in any terminal:

e.g: 

```bash
hello world 
```

## Test with netcat
Please refer to the `README.md` file in the `tests` folder.

