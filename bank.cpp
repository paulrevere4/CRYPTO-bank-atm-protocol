#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <map>
#include <sstream>
#include <vector>
#include <pthread.h>
#include <utility>

#include <crypto++/rsa.h>
#include <crypto++/osrng.h>
#include <crypto++/base64.h>
#include <crypto++/files.h>

#include "encrypt_decrypt.h"

#define BUFFER_SIZE = 1024

using namespace CryptoPP;
using namespace std;

typedef struct arg {
	int port;
	map<string, int> *bal;
	pthread_mutex_t* lock;
} arg;

// pair<RSA::PrivateKey, RSA::PublicKey>
// getNewKeys();

// string
// hash_and_encrypt(RSA::PublicKey publicKey, string plaintext);

// string
// decrypt(RSA::PrivateKey privateKey, string ciphertext);

// string
// get_message_wout_hash(string message_w_hash);

// bool
// verify_message(string message_w_hash);



void* listenPort(void* arguments)
{
  arg* args = (arg*)arguments;
  int PORT_TO_PROXY = args->port;
  map<string, int> *balances = args->bal;
  
  int listenfd = 0, connfd = 0, n = 0;
  struct sockaddr_in serv_addr;

  char sendBuff[1025], recvBuff[1025];

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  memset(&serv_addr, '0', sizeof(serv_addr));
  memset(sendBuff, '0', sizeof(sendBuff));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(PORT_TO_PROXY); 

  bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

  listen(listenfd, 10);
  
  while (1)
  {
    //cout << "listening..." << endl;
    connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

    bzero(recvBuff,1024);
    n = recv(connfd, &recvBuff, 1023, 0);
    // printf("Recd: %d bytes\n", n);
    // printf("Received message: %s\n", recvBuff);

    string command(recvBuff); 

    if ( command == "publickeyrequest" )
    {

      // cout << "Received request for public key" << endl;
      pair<RSA::PrivateKey, RSA::PublicKey>keys = getNewKeys();
      RSA::PrivateKey privateKey = keys.first;
      RSA::PublicKey publicKey = keys.second;

      string spki;
      StringSink ss(spki);

      // Use Save to DER encode the Subject Public Key Info (SPKI)
      publicKey.Save(ss);
      // std::cout << "publicKeyString: \n" << spki << endl;

      // snprintf(sendBuff, sizeof(sendBuff), "%s\n", spki.data());
      for ( unsigned int i = 0; i < spki.size(); i++ ) {
        sendBuff[i] = spki.data()[i];
      }
      // write(1, sendBuff, spki.size());
      write(connfd, sendBuff, spki.size());

      // printf("WAITING FOR A RESPONSE WITH DATA\n");
      //connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
      bzero(recvBuff,1024);
      n = recv(connfd, &recvBuff, 1023, 0);
      // printf("encrypted data:\n");
      // write(1, recvBuff, 256);
      string enc_msg = "";
      for ( unsigned int i = 0; i < 256; i++ ) {
        enc_msg += recvBuff[i];
      }
      // printf("\n");
      string plaintext = decrypt(privateKey, enc_msg);
      // printf("\n");

      string message = get_message_wout_hash(plaintext);

      if ( !verify_message(plaintext) )
      {
        cout << "HACKER!" << endl;
        cout << "DIE DIE DIE DIE!" << endl;
      }

      //========================================

      






      // cout << message << endl;
      strcpy(recvBuff, message.c_str());
      vector<string> sections;
      stringstream sstream(recvBuff);
      string split;

      while(getline(sstream,split,'.'))
      {
        sections.push_back(split);
      }

      string output;

      string username = sections[0];
      
      pthread_mutex_lock(args->lock);
      
      bool validUsername = (balances->find(username) != balances->end());

      if (!validUsername)
      {
        cout << "Your username has been compromised. Aborting session" << endl;
        break;
      }
      else if (sections[1] == "Balance")
      {
        output = "Your balance is ";
      }
      else if (sections[1] == "Withdraw"){
        string amount = sections[2];
        if (amount.size() > 10)
        {
          cout << "ERROR: Maximum account balance is 2000000000" << endl;
        }
        else if (amount.size() == 10 && (amount[0] != '0' &&  amount[0] != '1' && amount[0] != '2'))
        {
          cout << "ERROR: Maximum account balance is 2000000000" << endl;
        }
        else if (amount.size() == 10 && amount[0] == '2' && (amount[1] != '0' || amount[2] != '0' || amount[3] != '0' || amount[4] != '0' ||
                amount[5] != '0' || amount[6] != '0' || amount[7] != '0' || amount[8] != '0' || amount[9] != '0'))
        {
          cout << "ERROR: Maximum account balance is 2000000000" << endl;       
        }
        else if (amount.size() == 1 && amount[0] == '0')
        {
          cout << "ERROR: The amount entered must be a positive integer" << endl;
        }
        else
        {
          int deposit = atoi(sections[2].c_str());
          if (deposit > (*balances)[username])
          {
            output = "Insufficient funds. Your balance is ";
          }
          else
          {
            (*balances)[username] -= deposit;
            output = "Withdraw successful. Your balance is ";
          }
        }
      }
      else if (sections[1] == "Transfer")
      {
        string amount = sections[2];
        if (amount.size() > 10)
        {
          cout << "ERROR: Maximum account balance is 2000000000" << endl;
        }
        else if (amount.size() == 10 && (amount[0] != '0' &&  amount[0] != '1' && amount[0] != '2'))
        {
          cout << "ERROR: Maximum account balance is 2000000000" << endl;
        }
        else if (amount.size() == 10 && amount[0] == '2' && (amount[1] != '0' || amount[2] != '0' || amount[3] != '0' || amount[4] != '0' ||
                amount[5] != '0' || amount[6] != '0' || amount[7] != '0' || amount[8] != '0' || amount[9] != '0'))
        {
          cout << "ERROR: Maximum account balance is 2000000000" << endl;       
        }
        else if (amount.size() == 1 && amount[0] == '0')
        {
          cout << "ERROR: The amount entered must be a positive integer" << endl;
        }
        else
        {
          int transfer = atoi(sections[3].c_str());
          if(transfer > (*balances)[username])
          {
            output = "Insufficient funds. Your balance is ";
          }
          else
          {
            string transfer_to = sections[2];
            bool found_transfer = (balances->find(transfer_to) != balances->end());
            
            if (found_transfer)
            {
              (*balances)[username] -= transfer;
              (*balances)[transfer_to] += transfer;
              output = "Transfer successful. Your balance is ";
            }
            
            else
            {
              output = "Unable to find target account. Please try again.\nYour balance is ";
            }
          }
        }
      }
      else
      {
        output = "Something went wrong. Please try again.\nYour balance is ";
      }

      // send public key request
      bzero(sendBuff,1024);
      string request = "publickeyrequest";
      strcpy(sendBuff,request.c_str());
      write(connfd, sendBuff, strlen(sendBuff));


      bzero(recvBuff,1024);
      n = read(connfd,recvBuff,1023);

      StringSource ssource((unsigned char*)recvBuff, 1023, true);

      RSA::PublicKey publicKeyAtm;
      publicKeyAtm.Load(ssource);

      snprintf(sendBuff, sizeof(sendBuff), "%s%d\n", output.c_str(), (*balances)[username]);
      string res_plaintext(sendBuff);

      string enc_res = hash_and_encrypt(publicKeyAtm, res_plaintext);
      bzero(sendBuff,1024);
      for ( unsigned int i = 0; i < enc_res.size(); i++ ) {
          sendBuff[i] = enc_res[i];
      }
      write(connfd, sendBuff, enc_res.size());

      // snprintf(sendBuff, sizeof(sendBuff), "%s%d\n", output.c_str(), (*balances)[username]);
      // write(connfd, sendBuff, strlen(sendBuff));
      
      pthread_mutex_unlock(args->lock);

      close(connfd);

    }

  }
}

int main(int argc, char** argv)
{

  if (argc != 2)
  {
    cout << "USAGE: " << argv[0] << " <listen-port>" << endl;
    exit(1);
  }

  const int PORT_TO_PROXY = atoi(argv[1]);

  cout << "Hello world! I am a bank!" << endl;
  cout << "I will listen on port " << PORT_TO_PROXY << endl;

  map<string, int> balances;
  balances["Alice"] = 100;
  balances["Bob"] = 50;
  balances["Eve"] = 0;
  
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  
  arg args;
  args.port = PORT_TO_PROXY;
  args.bal = &balances;
  args.lock = &mutex;
  
  pthread_t tid;
  int rc = pthread_create(&tid, NULL, listenPort, (void*)&args);
			
	if (rc != 0)
	{
		printf("THREAD %u: ERROR: Could not create child thread\n", (unsigned)pthread_self());
		return -1;
	}

  while(1)
  {

    cout << "----------------------------" << endl;
    cout << "Enter one of the following:" << endl;
    cout << "Balance <username>" << endl;
    cout << "Deposit <username> <amount>" << endl;
    cout << "Exit" << endl;
    cout << "----------------------------" << endl;

    string command, username, amount;
    cin >> command;

    if (command == "Balance")
    {

      cin >> username;

      string leftover;
      getline(cin, leftover);

      if (balances.find(username) == balances.end())
      {
        cout << "ERROR: Invalid user" << endl;
      }
      else
      {
        cout << username << " has a balance of " << balances[username] << endl;
      }

    }

    else if (command == "Deposit")
    {
      cin >> username >> amount;
      string leftover;
      getline(cin, leftover);

      if (balances.find(username) == balances.end())
      {
        cout << "ERROR: Invalid user" << endl;
      }
      else
      {
        bool integer = true;
        for (unsigned int i=0;i<amount.size();i++)
        {
          if (!isdigit(amount[i]))
          {
            integer = false;
          }
        }
        if (!integer)
        {
          cout << "ERROR: The amount entered must be a positive integer" << endl;
        }
        else
        {
          if (amount.size() > 10)
          {
            cout << "ERROR: Maximum account balance is 2000000000" << endl;
          }
          else if (amount.size() == 10 && (amount[0] != '0' &&  amount[0] != '1' && amount[0] != '2'))
          {
            cout << "ERROR: Maximum account balance is 2000000000" << endl;
          }
          else if (amount.size() == 10 && amount[0] == '2' && (amount[1] != '0' || amount[2] != '0' || amount[3] != '0' || amount[4] != '0' ||
                   amount[5] != '0' || amount[6] != '0' || amount[7] != '0' || amount[8] != '0' || amount[9] != '0'))
          {
            cout << "ERROR: Maximum account balance is 2000000000" << endl;       
          }
          else
          {
            int deposit = atoi(amount.c_str());
            if (deposit == 0)
            {
              cout << "ERROR: The amount entered must be a positive integer" << endl;
            }
            else
            {
              if (balances[username] + deposit > 2000000000)
              {
                cout << "ERROR: Maximum account balance is 2000000000" << endl;
              }
              else
              {
                balances[username] += deposit;
                cout << "Deposit successful. " << username << " now has a balance of " << balances[username] << endl;
              }
            }
          }
        }
      }
    }
    
    else if (command == "Exit")
		{
			
			string leftover;
			getline(cin, leftover);
			
			cout << "Thank you for visiting" << endl;
			break;
		}

    else
    {
      string leftover;
      getline(cin, leftover);
      cout << "ERROR: Invalid command" << endl;
    }

    

  }

  pthread_cancel(tid);

  return 0;
}
