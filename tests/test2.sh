echo "test2: test /known_hosts return correct port"
echo "expected: 127.0.0.1:5001,\n"

# node A
echo "Starting node A on port 5000..."
./build/node 5000 &
A=$!
sleep 1

# node B
echo "Starting node B on port 5001, connecting to node A..."
./build/node 5001 127.0.0.1 5000 &
B=$!
sleep 2

echo "Asking 5000 for known_hosts ..."
response=$(printf "/known_hosts\n" | nc -w 1 127.0.0.1 5000)
echo "response: $response"

echo ""
if echo "$response" | grep -q "5001"; then
    echo "PASS"
else
    echo "FAIL: known_hosts missing or wrong port"
fi

kill $A $B