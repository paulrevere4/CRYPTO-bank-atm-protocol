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
	cout << "Hello world! I am an ATM!" << endl;
	
	if (argc != 2)
	{
		cout << "USAGE: ./atm.exe <connect-port>" << endl;
		return 1;
	}
	
	// Connecting to the bank
	
	int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    
    portno = atoi(argv[1]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sockfd < 0)
    {
    	perror("ERROR opening socket");
    }
        
    server = gethostbyname("localhost");
    cout << server << endl;
    
    if (server == NULL)
    {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,server->h_length);
    serv_addr.sin_port = htons(portno);
    
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    {
        perror("ERROR connecting");
    }
    
    // Now connected to the bank
	


	string users [3] = {"Alice", "Bob", "Eve"};
	map<string, string> pins;

	for (int i = 0; i < 3; i++)
	{
		ifstream user((users[i]+".card").c_str());
		string pin;
		user >> pin;

		pins[users[i]] = pin;
	}

	map<string, string>::iterator it;
	for (it = pins.begin(); it != pins.end(); it++)
	{
		cout << it->first << ": " << it->second << endl;
	}




	bool loggedIn = false;
	string username;

	while (true)
	{
		cout << "----------------------------" << endl;
		cout << "Enter one of the following:" << endl;
		cout << "Login <username>" << endl;
		cout << "Balance" << endl;
		cout << "Withdraw <amount>" << endl;
		cout << "Transfer <amount> <username>" << endl;
		cout << "Logout" << endl;
		cout << "Exit" << endl;
		cout << "----------------------------" << endl;

		string command;
		cin >> command;
		
		// These take place locally in the ATM

		if (command == "Login")
		{
			cin >> username;

			if (pins.find(username) == pins.end())
			{
				cout << "ERROR: Invalid user" << endl;
			}
			else
			{
				cout << "Enter your PIN: ";
				string pin;
				cin >> pin;
				if (pins[username] == pin)
				{
					cout << "Logging in " << username << endl;
					loggedIn = true;
				}
				else
				{
					cout << "ERROR: Incorrect PIN" << endl;
				}
			}
		}
		
		else if (command == "Logout")
		{
			if (loggedIn)
			{
				cout << "Logging out " << username << endl;
				loggedIn = false;
				username = "";
			}
			else
			{
				cout << "ERROR: Not logged in" << endl;
			}
		}

		else if (command == "Exit")
		{
			cout << "Thank you for visiting" << endl;
			break;
		}
		
		// These require messages to be sent to the bank

		else
		{
			if (!loggedIn)
			{
				cout << "ERROR: Not logged in" << endl;
			}

			else
			{
				if (command == "Balance")
				{
					bzero(buffer,256);
					string msg = username + "Balance";
					int i;
					for (i = 0; i < msg.size(); i++)
					{
						buffer[i] = msg[i];
					}
					msg[i] = '\0';
					n = write(sockfd,buffer,strlen(buffer));
    
    				if (n < 0) 
    				{
         				perror("ERROR writing to socket");
    				}
    
    				bzero(buffer,256);
    				n = read(sockfd,buffer,255);
    
    				if (n < 0)
    				{
         				perror("ERROR reading from socket");
    				}
    
    				printf("%s\n",buffer);
					//cout << "Printing " << username << "'s balance" << endl;
				}

				if (command == "Withdraw")
				{
					int amount;
					cin >> amount;
					cout << "Withdrawing " << amount << " dollars from " << username << "'s account" << endl;
				}

				if (command == "Transfer")
				{
					int amount;
					string recipient;
					cin >> amount >> recipient;
					if (pins.find(recipient) != pins.end())
					{
						cout << "Transferring " << amount << " dollars from " << username << " to " << recipient << endl;
					}
					else
					{
						cout << "ERROR: Invalid user" << endl;
					}
				}

				
			}
		}
		cout << endl;
	}
	close(sockfd);
	return 0;
}
