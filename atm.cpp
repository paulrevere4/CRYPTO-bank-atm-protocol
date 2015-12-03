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

#include <crypto++/rsa.h>
#include <crypto++/osrng.h>
#include <crypto++/base64.h>
#include <crypto++/files.h>

#include "encrypt_decrypt.h"

using namespace std;

int main(int argc, char** argv)
{
	cout << "Hello world! I am an ATM!" << endl;
	
	if (argc != 2)
	{
		cout << "USAGE: ./atm.exe <connect-port>" << endl;
		return 1;
	}

	int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[1024];
    
	string users [3] = {"Alice", "Bob", "Eve"};
	map<string, string> pins;

	for (int i = 0; i < 3; i++)
	{
		ifstream user((users[i]+".card").c_str());
		string pin;
		user >> pin;

		pins[users[i]] = pin;
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

			if (loggedIn)
			{
				cout << "ERROR: " << username << " is already logged in" << endl;
			}

			else
			{

				cin >> username;
				
				string leftover;
				getline(cin, leftover);

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
		}
		
		else if (command == "Logout")
		{
			string leftover;
			getline(cin, leftover);
			
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
			
			string leftover;
			getline(cin, leftover);
			
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
				// printf("command: %s\n", command);
				if (command == "Balance")
				{
					
					string leftover;
					getline(cin, leftover);
					
					portno = atoi(argv[1]);
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
    
    				bzero((char *) &serv_addr, sizeof(serv_addr));
    				serv_addr.sin_family = AF_INET;
    				bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,server->h_length);
    				serv_addr.sin_port = htons(portno);
    
    				if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    				{
        				perror("ERROR connecting");
    				}

    				bzero(buffer,1024);
					string request = "publickeyrequest";
					strcpy(buffer,request.c_str());

					n = write(sockfd,buffer,strlen(buffer));


					bzero(buffer,1024);
    				n = read(sockfd,buffer,1023);

    				StringSource ss((unsigned char*)buffer, 1023, true);

    				RSA::PublicKey publicKeyBank;
					publicKeyBank.Load(ss);

					string msg = username + ".Balance";
					string enc_msg = hash_and_encrypt(publicKeyBank, msg);

					bzero(buffer,1024);
					for ( unsigned int i = 0; i < enc_msg.size(); i++ ) {
				        buffer[i] = enc_msg[i];
				    }
					n = write(sockfd, buffer, enc_msg.size());
					if (n < 0) 
    				{
         				perror("ERROR writing to socket");
    				}

    				bzero(buffer,1024);
    				n = read(sockfd,buffer,1023);
    				string command(buffer);
    				if ( command != "publickeyrequest")
    				{
    					cout << "Tampering detected" << endl;
        				exit(1);
    				} 

    				pair<RSA::PrivateKey, RSA::PublicKey>keys = getNewKeys();
					RSA::PrivateKey privateKeyAtm = keys.first;
					RSA::PublicKey publicKeyAtm = keys.second;

					string spki;
					StringSink ssink(spki);

					publicKeyAtm.Save(ssink);

					bzero(buffer,1024);
					for ( unsigned int i = 0; i < spki.size(); i++ ) {
						buffer[i] = spki.data()[i];
					}

					write(sockfd, buffer, spki.size());

					bzero(buffer,1024);

					n = recv(sockfd, &buffer, 1023, 0);

					string enc_res = "";
					for ( unsigned int i = 0; i < 256; i++ ) {
					enc_res += buffer[i];
					}

					string res_plaintext = decrypt(privateKeyAtm, enc_res);

					string res_message = get_message_wout_hash(res_plaintext);


					if ( !verify_message(res_plaintext) )
					{
						cout << "Tampering detected" << endl;
        				exit(1);
					}

    				cout << res_message << endl;
    
    				if (n < 0) 
    				{
         				perror("ERROR writing to socket");
    				}
    				
    				close(sockfd);
				}

				else if (command == "Withdraw")
				{
					string amount;
					cin >> amount;
					
					string leftover;
					getline(cin, leftover);
					
					bool integer = true;
					for (int i = 0; i < amount.size(); i++)
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
          				else if (amount.size() == 1 && amount[0] == '0')
          				{
          					cout << "ERROR: The amount entered must be a positive integer" << endl;
          				}
						else
						{
							
							portno = atoi(argv[1]);
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
		    
		    				bzero((char *) &serv_addr, sizeof(serv_addr));
		    				serv_addr.sin_family = AF_INET;
		    				bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,server->h_length);
		    				serv_addr.sin_port = htons(portno);
		    
		    				if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
		    				{
		        				perror("ERROR connecting");
		    				}

		    				bzero(buffer,1024);
							string request = "publickeyrequest";
							strcpy(buffer,request.c_str());

							n = write(sockfd,buffer,strlen(buffer));

							bzero(buffer,1024);
		    				n = read(sockfd,buffer,1023);

		    				StringSource ss((unsigned char*)buffer, 1023, true);

		    				RSA::PublicKey publicKey;
							publicKey.Load(ss);

							string msg = username + ".Withdraw." + amount;
							string enc_msg = hash_and_encrypt(publicKey, msg);

							bzero(buffer,1024);
							for ( unsigned int i = 0; i < enc_msg.size(); i++ ) {
						        buffer[i] = enc_msg[i];
						    }
							n = write(sockfd, buffer, enc_msg.size());
							if (n < 0) 
		    				{
		         				perror("ERROR writing to socket");
		    				}

		    				//================================

				    		bzero(buffer,1024);
		    				n = read(sockfd,buffer,1023);
		    				string command(buffer);
		    				if ( command != "publickeyrequest")
		    				{
		    					cout << "Tampering detected" << endl;
        						exit(1);
		    				} 

		    				pair<RSA::PrivateKey, RSA::PublicKey>keys = getNewKeys();
							RSA::PrivateKey privateKeyAtm = keys.first;
							RSA::PublicKey publicKeyAtm = keys.second;

							string spki;
							StringSink ssink(spki);

							publicKeyAtm.Save(ssink);

							bzero(buffer,1024);
							for ( unsigned int i = 0; i < spki.size(); i++ ) {
								buffer[i] = spki.data()[i];
							}

							write(sockfd, buffer, spki.size());

							bzero(buffer,1024);
							n = recv(sockfd, &buffer, 1023, 0);

							string enc_res = "";
							for ( unsigned int i = 0; i < 256; i++ ) {
							enc_res += buffer[i];
							}

							string res_plaintext = decrypt(privateKeyAtm, enc_res);

							string res_message = get_message_wout_hash(res_plaintext);

							if ( !verify_message(res_plaintext) )
							{
								cout << "Tampering detected" << endl;
        						exit(1);
							}

		    				cout << res_message << endl;
		    
		    				if (n < 0) 
		    				{
		         				perror("ERROR writing to socket");
		    				}

	    				//==========================================================
    				
    						close(sockfd);
						}
					}
				}

				else if (command == "Transfer")
				{
					string amount;
					string recipient;
					cin >> amount >> recipient;
					
					string leftover;
					getline(cin, leftover);
					
					bool integer = true;
					for (int i = 0; i < amount.size(); i++)
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
          				else if (amount.size() == 1 && amount[0] == '0')
          				{
          					cout << "ERROR: The amount entered must be a positive integer" << endl;
          				}
          				else
          				{
					
							if (pins.find(recipient) != pins.end())
							{
								portno = atoi(argv[1]);
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
			    
			    				bzero((char *) &serv_addr, sizeof(serv_addr));
			    				serv_addr.sin_family = AF_INET;
			    				bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,server->h_length);
			    				serv_addr.sin_port = htons(portno);
			    
			    				if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
			    				{
			        				perror("ERROR connecting");
			    				}

			    				bzero(buffer,1024);
								string request = "publickeyrequest";
								strcpy(buffer,request.c_str());

								n = write(sockfd,buffer,strlen(buffer));

								bzero(buffer,1024);
			    				n = read(sockfd,buffer,1023);

			    				StringSource ss((unsigned char*)buffer, 1023, true);

			    				RSA::PublicKey publicKey;
								publicKey.Load(ss);

								string msg = username + ".Transfer." + recipient + '.' + amount;

								string enc_msg = hash_and_encrypt(publicKey, msg);

								bzero(buffer,1024);
								for ( unsigned int i = 0; i < enc_msg.size(); i++ ) {
							        buffer[i] = enc_msg[i];
							    }
								n = write(sockfd, buffer, enc_msg.size());
								if (n < 0) 
			    				{
			         				perror("ERROR writing to socket");
			    				}

			    				//================================

					    		bzero(buffer,1024);
			    				n = read(sockfd,buffer,1023);
			    				string command(buffer);
			    				if ( command != "publickeyrequest")
			    				{
			    					cout << "Tampering detected" << endl;
        							exit(1);
			    				} 

			    				pair<RSA::PrivateKey, RSA::PublicKey>keys = getNewKeys();
								RSA::PrivateKey privateKeyAtm = keys.first;
								RSA::PublicKey publicKeyAtm = keys.second;

								string spki;
								StringSink ssink(spki);

								publicKeyAtm.Save(ssink);

								bzero(buffer,1024);
								for ( unsigned int i = 0; i < spki.size(); i++ ) {
									buffer[i] = spki.data()[i];
								}

								write(sockfd, buffer, spki.size());

								bzero(buffer,1024);
								n = recv(sockfd, &buffer, 1023, 0);

								string enc_res = "";
								for ( unsigned int i = 0; i < 256; i++ ) {
									enc_res += buffer[i];
								}

								string res_plaintext = decrypt(privateKeyAtm, enc_res);

								string res_message = get_message_wout_hash(res_plaintext);

								if ( !verify_message(res_plaintext) )
								{
									cout << "Tampering detected" << endl;
        							exit(1);
								}

			    				cout << res_message << endl;
			    
			    				if (n < 0) 
			    				{
			         				perror("ERROR writing to socket");
			    				}
				    			
	    					
	    						close(sockfd);
							}
							else
							{
								cout << "ERROR: Invalid user" << endl;
							}
          				}
					}
				}
				
				else {
					
					string leftover;
					getline(cin, leftover);
					
					cout << "ERROR: Invalid command" << endl;
				}

			}
		}
		cout << endl;
	}
	
	return 0;
}
