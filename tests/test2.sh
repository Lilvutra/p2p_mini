echo "test2: test /known_hosts return correct port"
echo "expected: 127.0.0.1:5001,\n"

# node A
echo "Starting node A on port 5001..."
./build/node 5001 &
A=$!
sleep 1

# node B
echo "Starting node B on port 5002, connecting to node A..."
./build/node 5002 127.0.0.1 5001 &
B=$!
sleep 2

echo "Asking 5001 for known_hosts ..."
response=$(printf "/known_hosts\n" | nc -w 2 127.0.0.1 5001) # response here means 
echo "response: $response"
# why response contains Hello:5000 along with HOSTS:127.0.0.1:5001? 

echo ""
if echo "$response" | grep -q "5001"; then
    echo "PASS"
else
    echo "FAIL: known_hosts missing or wrong port"
fi

kill $A $B