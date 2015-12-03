// encrypt_decrypt.cpp

// DEPENDS ON libcrypto++-dev 
// apt-get install libcrypto++-dev 
// compile with: g++ encypt_decrypt.cpp -lcryptopp

#include <iostream>
#include <utility>
#include <cassert>

#include <crypto++/rsa.h>
#include <crypto++/osrng.h>
#include <crypto++/base64.h>
#include <crypto++/files.h>

#include "encrypt_decrypt.h"

using namespace CryptoPP;
using namespace std;

// namespace enc{

pair<RSA::PrivateKey, RSA::PublicKey>
getNewKeys()
{
  pair<RSA::PrivateKey, RSA::PublicKey> retVal;

  AutoSeededRandomPool randomNumberGenerator;
  InvertibleRSAFunction params;
  params.Initialize(randomNumberGenerator, 2048);

  RSA::PrivateKey privateKey(params);
  RSA::PublicKey publicKey(params);

  retVal.first = privateKey;
  retVal.second = publicKey;

  return retVal;
}

string
hash_and_encrypt(RSA::PublicKey publicKey, string plaintext)
{
  string retVal;

  unsigned char hashed[21];
  hashed[20] = '\0';
  SHA().CalculateDigest(hashed, (byte *)plaintext.c_str(), plaintext.length());
  string hash_and_plaintext = string((char *)hashed) + plaintext;

  AutoSeededRandomPool randomNumberGenerator;

  RSAES_OAEP_SHA_Encryptor e(publicKey);
  
  StringSource ss1(hash_and_plaintext, 
      true,
      new PK_EncryptorFilter(randomNumberGenerator, 
          e, 
          new StringSink(retVal)) // PK_EncryptorFilter
      ); // StringSource

  return retVal;
}

string
decrypt(RSA::PrivateKey privateKey, string ciphertext)
{
  string retVal;

  AutoSeededRandomPool randomNumberGenerator;

  RSAES_OAEP_SHA_Decryptor d(privateKey);

  StringSource ss2(ciphertext, 
      true,
      new PK_DecryptorFilter(randomNumberGenerator, 
          d, 
          new StringSink(retVal)) // PK_DecryptorFilter
      ); // StringSource

  return retVal;
}

string
get_message_wout_hash(string message_w_hash)
{
  string retVal = "";
  for ( unsigned int i = 20; i < message_w_hash.length(); i++ )
  {
    retVal += message_w_hash[i];
  }
  return retVal;
}

bool
verify_message(string message_w_hash)
{
  string message_wout_hash = get_message_wout_hash(message_w_hash);

  unsigned char hashed[21];
  hashed[20] = '\0';
  SHA().CalculateDigest(hashed, (byte *)message_wout_hash.c_str(), message_wout_hash.length());
  
  return string((char *)hashed)+message_wout_hash == message_w_hash;
}

// int main()
// {
  

//   //test function
//   pair<RSA::PrivateKey, RSA::PublicKey> keys = getNewKeys();

//   RSA::PrivateKey privateKey = keys.first;
//   RSA::PublicKey publicKey = keys.second;

//   string plain = "Alice.Transfer.Bob.XXXXXXXXXX";
//   // string plain = "Bob.Balance";
//   cout << "plaintext: " << plain << endl;

//   unsigned char hashed[21];
//   hashed[20] = '\0';
//   cout << "SHA_digest_size: " << SHA::DIGESTSIZE << endl;
//   SHA().CalculateDigest(hashed, (byte *)plain.c_str(), plain.length());
  
//   printf("hashed plaintext: %s\n", hashed);
//   string hash_and_plaintext = string((char *)hashed) + plain;
//   cout << "hash_and_plaintext: " << hash_and_plaintext << endl;


//   //Encryption
//   string ciphertext = hash_and_encrypt(publicKey, plain);
//   cout << "ciphertext: \n" << ciphertext << endl;
//   cout << "ciphertext length: " << ciphertext.length() << endl;

//   // Decryption
//   string decryptedPlaintext = decrypt(privateKey, ciphertext);
//   cout << "recovered text: " << decryptedPlaintext << endl;
//   assert(decryptedPlaintext == hash_and_plaintext);

//   //get the message without the hash
//   string message_wout_hash = get_message_wout_hash(decryptedPlaintext);
//   cout << "plaintext without hash: " << message_wout_hash << endl;
//   assert(message_wout_hash == plain);

//   assert(verify_message(decryptedPlaintext));
//   cout << "message verified\n\n" << endl;

//   // Temporaries
//   string spki;
//   StringSink ss(spki);

//   // Use Save to DER encode the Subject Public Key Info (SPKI)
//   publicKey.Save(ss);
//   std::cout << "publicKeyString: \n" << spki << endl;
//   std::cout << "publicKeyString.size(): \n" << spki.size() << endl;
//   // printf("publicKeyString: \n%s\n", spki.data());




//   return 0;
// }

// }

