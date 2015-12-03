echo "Building..."
g++ atm.cpp -o atm
g++ proxy.cpp -o proxy
g++ bank.cpp -o bank -pthread

echo "Done Building, run the following commands"
echo "./atm <port1>"
echo "./proxy <port1> <port2>"
echo "./bank <port2>"
