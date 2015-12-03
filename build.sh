echo "Building..."
g++ atm.cpp encrypt_decrypt.cpp -lcryptopp -o atm
g++ bank.cpp encrypt_decrypt.cpp -pthread -lcryptopp -o bank
g++ proxy.cpp -o proxy

echo "Done Building, run the following commands"
echo "./atm <port1>"
echo "./bank <port2>"
echo "./proxy <port1> <port2>"
