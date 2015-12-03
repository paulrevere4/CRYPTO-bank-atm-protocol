echo ""
echo "Building..."
echo "[+] Building atm:"
g++ atm.cpp encrypt_decrypt.cpp -lcryptopp -o atm
echo "[!] atm built!"
echo ""
echo "[+] Building bank:"
g++ bank.cpp encrypt_decrypt.cpp -pthread -lcryptopp -o bank
echo "[!] bank built!"
echo ""
echo "[+] Building proxy:"
g++ proxy.cpp -o proxy
echo "[!] proxy built!"
echo ""
echo "Done Building, run the following commands on separate terminals"
echo "$ ./atm <port1>"
echo "$ ./bank <port2>"
echo "$ ./proxy <port1> <port2>"
echo ""
