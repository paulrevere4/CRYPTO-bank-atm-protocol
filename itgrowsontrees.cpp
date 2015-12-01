#include <iostream>
using namespace std;

int main() {

	cout << "----------------------" << endl;
	cout << "Welcome to ATM 420.69!" << endl;
	cout << "----------------------" << endl;
	cout << "First, you must pass a citizenship test." << endl;
	cout << "What's the best country in the world?" << endl;

	string input;
	cin >> input;

	if (input == "USA" 						||
		input == "US" 						||
		input == "United States" 			||
		input == "United States of America" ||
		input == "America" 					||
		input == "'merica") {

		cout << "Congratulations, you've passed! How much money would you like to withdraw?" << endl;

		long long amount;
		cin >> amount;

		cout << "Successfully withdrawn " << amount << " dollars. Please take your cash." << endl;
		cout << "The United States national debt is now " << (long long)18532854382822+amount << " dollars." <<endl;
	}

	else {
		cout << "Sorry, that's incorrect. You should probably go join ISIS or something." << endl;
	}

}