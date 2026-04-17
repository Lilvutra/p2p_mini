# P2P 

Simple p2p system with each node:
- listens for peers
- connects to peers
- forwards messages

---

## 2. How to test

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

