// encrypt_decrypt.cpp

// DEPENDS ON libcrypto++-dev 
// apt-get install libcrypto++-dev 
// compile with: g++ encypt_decrypt.cpp -lcryptopp

#include <iostream>
#include <utility>

#include <crypto++/rsa.h>
#include <crypto++/osrng.h>
#include <crypto++/base64.h>
#include <crypto++/files.h>

using namespace CryptoPP;
using namespace std;

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
encrypt(RSA::PublicKey publicKey, string plaintext)
{
  string retVal;

  AutoSeededRandomPool randomNumberGenerator;

  RSAES_OAEP_SHA_Encryptor e(publicKey);
  
  StringSource ss1(plaintext, 
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

int main()
{
  //test function
  pair<RSA::PrivateKey, RSA::PublicKey> keys = getNewKeys();

  RSA::PrivateKey privateKey = keys.first;
  RSA::PublicKey publicKey = keys.second;

  // string plain = "Alice.Transfer.Bob.XXXXXXXXXX";
  string plain = "Bob.Balance";
  cout << "plaintext: " << plain << endl;

  //Encryption
  string ciphertext = encrypt(publicKey, plain);
  cout << "ciphertext: " << ciphertext << endl;
  cout << "ciphertext length: " << ciphertext.length() << endl;

  // Decryption
  string decryptedPlaintext = decrypt(privateKey, ciphertext);
  cout << "recovered text: " << decryptedPlaintext << endl;

  return 0;
}

