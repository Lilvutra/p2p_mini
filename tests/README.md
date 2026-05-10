# How to test

Clean the test environment
```bash
pkill node
```
```bash
chmod +x tests/test1.sh
./tests/test1.sh
```

# Test cases:
`test1.sh`: test handshake, if the message send to correct port

`test2.sh`: test peer connection

`test3.sh`: test `known_hosts` for 3 nodes

`test4.sh`: test `boadcast`