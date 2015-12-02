// encrypt_decrypt.cpp

// DEPENDS ON libcrypto++-dev 
// apt-get install libcrypto++-dev 

#include <iostream>

#include <crypto++/rsa.h>
#include <crypto++/osrng.h>
#include <crypto++/base64.h>
#include <crypto++/files.h>

using namespace CryptoPP;
using namespace std;

int main()
{

  AutoSeededRandomPool randomNumberGenerator;
  InvertibleRSAFunction params;
  params.Initialize(randomNumberGenerator, 2048);

  RSA::PrivateKey privateKey(params);
  RSA::PublicKey publicKey(params);

  string plain = "Alice.Transfer.Bob.XXXXXXXXXX";
  string cipher, recovered;

  cout << "plaintext: " << plain << endl;

  //Encryption
  RSAES_OAEP_SHA_Encryptor e(publicKey);

  StringSource ss1(plain, 
      true,
      new PK_EncryptorFilter(randomNumberGenerator, 
          e, 
          new StringSink(cipher)) // PK_EncryptorFilter
      ); // StringSource

  cout << "ciphertext: " << cipher << endl;

  // Decryption
  RSAES_OAEP_SHA_Decryptor d(privateKey);

  StringSource ss2(cipher, 
      true,
      new PK_DecryptorFilter(randomNumberGenerator, 
          d, 
          new StringSink(recovered)) // PK_DecryptorFilter
      ); // StringSource

  cout << "recovered text: " << recovered << endl;





  return 0;
}

