echo "test2: test /known_hosts return correct port"
echo "expected: 127.0.0.1:5001,"

# node A
./node 5000 &
A=$!
sleep 1

# node B
./node 5001 127.0.0.1 5000 &
B=$!
sleep 2

echo "Asking 5000 for known_hosts ..."
response=$(echo -ne "/known_hosts\n" | nc -q1 127.0.0.1 5000)
echo "response: $response"

echo ""
if echo "$response" | grep -q "5001"; then
    echo "PASS"
else
    echo "FAIL: known_hosts missing or wrong port"
fi
 
kill $A $B