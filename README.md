# P2P 

Simple p2p system with each node:
- listens for peers
- connects to peers
- forwards messages

---

## Network Topology
The following diagram illustrates how the main() function initializes the core threads and how those threads subsequently spawn peer handlers 

![Thread Hierarchy and Data Flow](p2p.drawio.svg)

## How to test

### Compile 

```bash
gcc p2p_node.c -o node -pthread
```

### Terminal 1

```bash
./node 5000
```
### Terminal 2
```bash
./node 5001 127.0.0.1 5000
```

### Terminal 3

```bash
./node 5002 127.0.0.1 5000
```

type something in any terminal:

e.g: 

```bash
hello world 
```

## Test with netcat
Clean the test environment
```bash
pkill node
```

Run the test
```bash
chmod +x test1.sh
./test1.sh
```

