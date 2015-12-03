#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <cstdlib>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
using namespace std;

int main(int argc, char** argv)
{

  if (argc != 3)
  {
    cout << "USAGE: " << argv[0] << " <listen-port> <send-port>" << endl;
    exit(1);
  }

  const int LISTEN_PORT = atoi(argv[1]);
  const int SEND_PORT = atoi(argv[2]);

  cout << "Hello world! I am a proxy!" << endl;
  // cout << "I will listen on port " << LISTEN_PORT << endl;
  // cout << "I send to port " << SEND_PORT << endl;

  int listenfd = 0, connfd = 0, n = 0;
  int sockfd = 0;
  struct sockaddr_in listen_serv_addr, send_serv_addr;
  struct hostent *server;

  char sendBuff[1025], recvBuff[1025];

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  memset(&listen_serv_addr, '0', sizeof(listen_serv_addr));
  memset(sendBuff, '0', sizeof(sendBuff));

  listen_serv_addr.sin_family = AF_INET;
  listen_serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  listen_serv_addr.sin_port = htons(LISTEN_PORT); 

  bind(listenfd, (struct sockaddr*)&listen_serv_addr, sizeof(listen_serv_addr)); 

  listen(listenfd, 10);

  while(1)
  {
    // cout << "listening..." << endl;

    connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);  
    if (sockfd < 0)
    {
      perror("ERROR opening socket");
    }

    server = gethostbyname("localhost");
      
    if (server == NULL)
    {
      fprintf(stderr,"ERROR, no such host\n");
      exit(0);
    }

    bzero((char *) &send_serv_addr, sizeof(send_serv_addr));
    send_serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&send_serv_addr.sin_addr.s_addr,server->h_length);
    send_serv_addr.sin_port = htons(SEND_PORT);

    if ( connect(sockfd, (struct sockaddr *)&send_serv_addr, sizeof(send_serv_addr)) < 0)
    {
        perror("ERROR connecting");
    }

    bzero(recvBuff,1024);
    bzero(sendBuff,1024);
    n = recv(connfd, &recvBuff, 1023, 0);
    // printf("Recd: %d bytes from ATM\n", n);
    // printf("Received message from ATM: \n");

    for ( int i = 0; i < n; i++ )
    {
      sendBuff[i] = recvBuff[i];
    }
    n = write(sockfd, sendBuff, n);
    // printf("Sent: %d bytes to BANK\n", n);

    //============================================

    bzero(recvBuff,1024);
    bzero(sendBuff,1024);
    n = read(sockfd, recvBuff, 1023);
    // printf("Read: %d bytes from BANK\n", n);
    // printf("Received message from BANK: \n");

    for ( int i = 0; i < n; i++ )
    {
      sendBuff[i] = recvBuff[i];
    }
    n = write(connfd, sendBuff, n);
    // printf("Sent: %d bytes to ATM\n", n);

    //============================================

    bzero(recvBuff,1024);
    bzero(sendBuff,1024);
    n = recv(connfd, &recvBuff, 1023, 0);
    // printf("Recd: %d bytes from ATM\n", n);
    // printf("Received message from ATM: \n");

    for ( int i = 0; i < n; i++ )
    {
      sendBuff[i] = recvBuff[i];
    }
    n = write(sockfd, sendBuff, n);
    // printf("Sent: %d bytes to BANK\n", n);

    //============================================

    bzero(recvBuff,1024);
    bzero(sendBuff,1024);
    n = read(sockfd, recvBuff, 1023);
    // printf("Read: %d bytes from BANK\n", n);
    // printf("Received message from BANK: \n");

    for ( int i = 0; i < n; i++ )
    {
      sendBuff[i] = recvBuff[i];
    }
    n = write(connfd, sendBuff, n);
    // printf("Sent: %d bytes to ATM\n", n);

    //============================================

    bzero(recvBuff,1024);
    bzero(sendBuff,1024);
    n = recv(connfd, &recvBuff, 1023, 0);
    // printf("Recd: %d bytes from ATM\n", n);
    // printf("Received message from ATM: \n");

    for ( int i = 0; i < n; i++ )
    {
      sendBuff[i] = recvBuff[i];
    }
    n = write(sockfd, sendBuff, n);
    // printf("Sent: %d bytes to BANK\n", n);

    //============================================

    bzero(recvBuff,1024);
    bzero(sendBuff,1024);
    n = read(sockfd, recvBuff, 1023);
    // printf("Read: %d bytes from BANK\n", n);
    // printf("Received message from BANK: \n");

    for ( int i = 0; i < n; i++ )
    {
      sendBuff[i] = recvBuff[i];
    }
    n = write(connfd, sendBuff, n);
    // printf("Sent: %d bytes to ATM\n", n);


    close(connfd);
    sleep(1);
  }


  return 0;
}
