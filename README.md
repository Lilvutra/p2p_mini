# P2P 

## Introduction
Simple p2p system with each node:
- listens for peers: receive incoming connections
- connects to peers: read message and broadcasts to peers.
- forwards messages: receive message and forwards to other peers
Each node in the network can send and receive messages directly to other nodes, no central server. If 1 node goes down, the others keep talking.

## How it works
### Peer discovery
When a new node connect to 1 node in the network. It will be automatically connect to the nodes that the node in the network has already known.

## Message protocol
Message follow these format:
* `Hello:5001` : share listening port after connecting
* `known_hosts` : request peer list that that node has known
* `HOSTS: ip:port, ip:port`: known peer list response
* `MSG:id|text`: chat message with unique id

## Loop prevention
Assign unique id for each message to prevent infinite message bounce.

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

