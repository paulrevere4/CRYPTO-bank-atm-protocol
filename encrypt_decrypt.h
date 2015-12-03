#ifndef ENCRYPT_DECRYPT_H
#define ENCRYPT_DECRYPT_H

#include <iostream>
#include <utility>
#include <cassert>

#include <crypto++/rsa.h>
#include <crypto++/osrng.h>
#include <crypto++/base64.h>
#include <crypto++/files.h>

using namespace CryptoPP;
using namespace std;

// namespace enc{

pair<RSA::PrivateKey, RSA::PublicKey>
getNewKeys();

string
hash_and_encrypt(RSA::PublicKey publicKey, string plaintext);

string
decrypt(RSA::PrivateKey privateKey, string ciphertext);

string
get_message_wout_hash(string message_w_hash);

bool
verify_message(string message_w_hash);

// }

#endif