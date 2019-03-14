#include <fstream>
#include <iostream>
#include <string>
#include <limits>
#include <windows.h>
using namespace std;

#define ADDUSER 1 
#define SETTIME 2 
#define SETPASS 3 
#define QUIT 4
void menu_display();
// admin menu functions
// <---add users and set the global pass
// edit time stamps
int main()
{
	int choice = -1;

	cout << "Restart the server after changes do not run it while this is active\n";

	while(choice != QUIT)
	{
		menu_display();
		cout << "Please enter a menu choice: ";
		cin >> choice;
		
		if(choice == ADDUSER)
		{
			cout << "Please enter the user details\n\n";
			fstream fp{"cred.txt", ios::out | ios::in | ios_base::app};
			
			string userid;
			string password;
			
			cout << "Please enter the name: ";
			
			cin >> userid;
			
			// read credit.txt and check that the username is not taken
			string cpass;
			string cname;
			int id;
			bool fail = false;
			
			do
			{
				fp.seekg(0, ios::beg);
				fail = false; // reset the indicator
				while(fp >> cname >> cpass >> id)
				{
					if(cname == userid)
					{
						cout << "Error user name is all ready taken please enter a new one: ";
						cin >> userid;
						fail = true;
						break;
					}
				}
			} 
			while(fail == true);
			
			fp.clear(); 
			
			
			cout << "Please enter the password: ";
			
			cin >> password;
			
			
			
			// eg a student user or a moderator that can pull the students work
			// will add groups for this later
			
			cout << "Please enter the access rights of the user\n";
			cout << "0 for a moderator\n"
				<< "1 for a student\n";
			
			int access_id = -1;
			
			
			cout << "? ";
			while(!(cin >> access_id))
			{
				cin.clear();
				cin.ignore(numeric_limits<streamsize>::max(), '\n');
				cout << "Please enter a number\n? ";
			}
			
			
			// write them out to cred.dat
			
			fp <<  '\n' << userid;
			fp << '\n' << password;
			fp << '\n' << access_id;
			
			// create the userdir
			CreateDirectory(userid.c_str(),NULL);
		    ofstream fpo {userid + "/user.dat", ios::out | ios::binary}; // write out first line which is time remaining;
			
			
			
			fpo << (72 * 60 * 60); // write out 72 hours in seconds
			
			fpo.close();
			fp.close();
		}
	
		else if(choice == SETTIME)
		{
			cout << "Enter the user id to change time for: \n";
			
			// lookup if there is a valid user and open the user.dat for the user
			
		}
		
		else if(choice == SETPASS)
		{
			cout << "Enter the new global password\n";
			
			fstream fp{"server.cfg", ios::out | ios::binary};
			
			// change the 1st line of the server cfg (the global pass)
			string pass;
			
			cin >> pass;
			fp << pass;
			fp.close();
		}
		
		else if(choice == QUIT)
		{
			cout << "Exiting program goodbye...\n";
		}
		
		
		else
		{
			cout << "Unknown menu choice\n";
		}
		
		cout << "\n\n\n";
		
	}
}





void menu_display() // <--- could overload this as a ostream var 
{				    // but not much point for a static menu
	cout << "1-Add a new user\n"
		<< "2-Set a users time\n"
		<< "3-Change the global server password\n"
		<< "4-Quit the program\n";
}