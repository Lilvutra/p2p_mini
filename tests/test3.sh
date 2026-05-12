echo "test3: test /known_hosts for 3 nodes"
echo "expected (from 5000 we get): 127.0.0.1:5001,127.0.0.1:5002,"

# node A
./build/node 5000 &
A=$!
sleep 0.5

# node B
./build/node 5001 127.0.0.1 5000 &
B=$!
sleep 1

# node c
./build/node 5002 127.0.0.1 5000 &
C=$!
sleep 1.5

echo "Asking node 5000 for known_hosts"
response=$(printf "/known_hosts\n" | nc -w 1 127.0.0.1 5000)
echo "response: $response"

echo ""
if echo "$response" | grep -q "5001" && echo "$response" | grep -q "5002"; then
    echo "PASS"
else
    echo "FAIL: missing peers in known_hosts"
fi

kill $A $B $C
