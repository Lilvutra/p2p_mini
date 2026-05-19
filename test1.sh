# node A
./node 5000 &
A=$!

# connect node B to A
./node 5001 127.0.0.1 5000 &
B=$!

# wait for connection to form
sleep 1
# send message
echo -ne "hello\n" | nc 127.0.0.1 5000
# wait for output
sleep 2

# kill nodes
kill $A $B