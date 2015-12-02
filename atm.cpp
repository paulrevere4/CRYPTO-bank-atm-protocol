#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <cstdlib>
using namespace std;

int main(int argc, char** argv)
{
	cout << "Hello world! I am an ATM!" << endl;

	if (argc != 2)
	{
		cout << "USAGE: ./atm.exe <connect-port>" << endl;
		return 1;
	}

	int port = atoi(argv[1]);

	cout << "Going to connect on port " << port << endl;

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

		else if (command == "Exit")
		{
			cout << "Thank you for visiting" << endl;
			break;
		}

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
					cout << "Printing " << username << "'s balance" << endl;
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

				if (command == "Logout")
				{
					cout << "Logging out " << username << endl;
					loggedIn = false;
					username = "";
				}
			}
		}
		cout << endl;
	}

	return 0;
}
