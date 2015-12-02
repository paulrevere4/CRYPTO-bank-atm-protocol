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

#define BUFFER_SIZE = 1024

using namespace std;

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

  // ==========================================================================

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

  while(1)
  {
    cout << "listening..." << endl;
    connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

    bzero(recvBuff,1024);
    n = recv(connfd, &recvBuff, 1023, 0);
    printf("Recd: %d bytes\n", n);
    printf("Received message: %s\n", recvBuff);

    vector<string> sections;
    stringstream ss(recvBuff);
    string split;

    while(getline(ss,split,'.'))
    {
      sections.push_back(split);
    }

    string username = sections[0];
    string output;

    if (sections[1] == "Balance")
    {
      output = "Your balance is ";
    }
    else if (sections[1] == "Withdraw"){
      int amount = atoi(sections[2].c_str());
      if (amount < 0)
      {
        output = "You must withdraw a positive amount. Your balance is ";
      }
      else if (amount > balances[username])
      {
        output = "Insufficient funds. Your balance is ";
      }
      else
      {
        balances[username] -= amount;
        output = "Withdraw successful. Your balance is ";
      }
    }
    else if (sections[1] == "Transfer")
    {
      int amount = atoi(sections[2].c_str());
      if (amount < 0)
      {
        output = "You must transfer a positive amount. Your balance is ";
      }
      else if(amount > balances[username])
      {
        output = "Insufficient funds. Your balance is ";
      }
      else
      {
        balances[username] -= amount;
        balances[sections[3]] += amount;
        output = "Transfer successful. Your balance is ";
      }
    }

    snprintf(sendBuff, sizeof(sendBuff), "%s%d\n", output.c_str(), balances[username]);
    write(connfd, sendBuff, strlen(sendBuff));

    close(connfd);
    sleep(1);
  }






  return 0;
}
