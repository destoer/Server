//https://docs.microsoft.com/en-us/windows/desktop/winsock/complete-client-code



// must link with Ws2_32 
#define _WIN32_WINNT 0x501 // keep windows happy and define the version we want to use

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <fstream>
#include <iomanip>
#include <string>
#include <limits>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <time.h>
#include "lib.h"
#include "Connection.h"
using namespace std;

void initClient(Connection_struct *c,char *ip,char *port);

#define LEN 31
typedef struct {
	char name[LEN];
	char password[LEN];
} Login;
// may be better pratice to use a namespace here
/* namespace choices
{
	enum myChoice
	{
		Quit = 1,
		CheckTime = 2,
	
	};
} */
enum class Choice {Logout = 1, CheckTime };


// add setting a note for the users maybye?
#define DOWNLOAD 2
#define QUIT 1


int main(int argc, char *argv[])
{
	
	Connection_struct c_tmp;
	char buffer[512] = {0};
	// should ensure that the port is in range and ip is as dotted qaud
	
	if(argc != 3)
	{
		cout << "usage: " << argv[0] << " <ip> <port>" << endl;
		return 1;
	}
	
	initClient(&c_tmp,argv[1],argv[2]); // ready to send following this
	Connection c{c_tmp};
try
{
	string username;
	string password;

	cout << "Please enter your username: ";
	cin >> setw(20) >> username; // setw limits input field to 20 characters
	cout << "Please enter your password: ";
	cin >> setw(20) >> password;

	c << username;
	c << password;
	
	int num = 0;
	c >> num;
	
	
	if(num == 1)
	{
		cout << "Login successful\n";
	}
	
	else 
	{
		cout << "Invalid username and or password" << endl;
		return 0;		
	}
	
	int access_id; c >> access_id;

	// student
	if(access_id == 1)
	{
		// create a student dir incase it does not exist 
		CreateDirectory("work/",NULL);
		char dirbuf[50] = "work/";
		strcpy(dirbuf,username.c_str());
		CreateDirectory(dirbuf,NULL);
		time_t t1 = time(NULL);
		
		
		// serve menu options 
		int choiceint = 2;
		Choice choice = static_cast<Choice>(choiceint);
		
		
		// takes two tries to register a choice and doesent exit
		// should not need a goto statement to break the loop
		while(choice != Choice::Logout)
		{
			
			cout << "\n\n\n1-Logout\n"
				<< "2-Get the remaing time\n";
			cout << "Enter your choice: ";

			cout << "? ";
			while(!(cin >> choiceint))
			{
					cin.clear();
					cin.ignore(numeric_limits<streamsize>::max(), '\n');
					cout << "Please enter a number\n? ";
			}

			Choice choice = static_cast<Choice>(choiceint);
			
			
			switch(choice)
			{
				case Choice::Logout: 
				{
					cout << "Logging out uploading files..." << endl;
					c << "Logout";

					time_t t2;
					t2 = time(NULL); // get system time
		
					
					cout << t1 << ' ' << t2 << ' ' << t2-t1 << '\n';
					t2 = t2 - t1; // subtract the end from initial to get the difference
					c << t2;
					
					// should configure this out of a text file
					// ideally or at install but that aint happening without admin
					// D:/coursework/work/ is where users work should be stored modify this as need be
					c.send_files("D:/coursework/work/" + username + "/");
					
					goto x; // <--- should not need to break out of the loop
					break;
				}
				
				case Choice::CheckTime:
				{
					// get the remaining time
					c << "Get time";
					
					
					// calc our current time 
					time_t t2;
					char buffer2[sizeof(time_t)];
					t2 = time(NULL); // get system time
					
					
					cout << t1 << ' ' << t2 << ' ' << t2-t1 << '\n';
					t2 = t2 - t1;
					c << t2;
					

					c >> t2;
					
					cout << "You have " << t2 << ' ' << "remaining\n";
					break;
				}
				
				default: cout << "error invalid menu choice\n"; break;
			}
				
		}
	}
	
	
	// mod access
	else if(access_id == 0)
	{
		int choice = 0;
		
		
		
		// takes two tries to register a choice and doesent exit
		// should not need a goto statement to break the loop
		while(choice != QUIT)
		{
			
			cout << "\n\n\n1-Logout\n"
				<< "2-Download a users file\n";
			cout << "Enter your choice: ";

			cout << "? ";
			while(!(cin >> choice))
			{
					cin.clear();
					cin.ignore(numeric_limits<streamsize>::max(), '\n');
					cout << "Please enter a number\n? ";
			}




			switch(choice)
			{
				case QUIT:
				{
					cout << "Goodbye!\n";
					
					// send the choice
					c << "Quit";
					break;
				}
					
				case DOWNLOAD:
				{
					cout << "Please enter a username to download files for\n";
					cout << "UserName: ";	
					c << "Download";
					string userdir;
					cin >> setw(20) >> userdir;
					
					c << userdir;
					
					// take the input
				/*	fflush(stdin);
					memset(buffer,0,LEN+1);
					fgets(buffer,LEN,stdin);
					buffer[strcspn(buffer, "\n")] = '\0';
					fflush(stdin); 
				*/
					// and send the username if it is valid the server will send back the files
					// it will handle a invalid name by dropping the client a proper error
					// instead of a general one would be more ideal....
					
					// recv the files
					c.recv_files("out2/" + userdir + "/");

					break;
				}
					
				default:
				{
					cout << "error invalid choice"; 
					break;
				}
			}
		}	
	}
	
	// invalid user access
	else
	{
		cout << "invalid user access";
	}
}
catch(exception &e)
{
		cout << "Exception detected closing client handler\n";
		exit(1);
}	
x:
	cout << "bye!" << endl;
	WSACleanup();	
	return 0;
}	



// probably should be init client but allow it
void initClient(Connection_struct *c,char *ip,char *port)
{
	int rc = WSAStartup(MAKEWORD(2,2), &c->wsaData);
	if(rc != 0)
	{
		cerr << "WSAStartup failed: " << rc << endl;
		exit(1);
	}
	
	ZeroMemory(&c->hints,sizeof(c->hints));
	c->hints.ai_family = AF_UNSPEC;
	c->hints.ai_socktype = SOCK_STREAM;
	c->hints.ai_protocol = IPPROTO_TCP;
	
	//resolve server address and port
	rc = getaddrinfo(ip,port,&c->hints, &c->result);
	if(rc != 0)
	{
		cerr << "getaddrinfo failed: " << rc << endl;
		WSACleanup();
		exit(1);
	}
	
	// attempt connection until one succeeds
	for(c->ptr=c->result; c->ptr != NULL; c->ptr=c->ptr->ai_next)
	{
		c->clientSocket = socket(c->ptr->ai_family, c->ptr->ai_socktype, c->ptr->ai_protocol);
		if(c->clientSocket == INVALID_SOCKET)
		{
			cerr << "socket failed: " << WSAGetLastError() << endl;
			WSACleanup();
			exit(1);
		}

		// connect to the server
		rc = connect(c->clientSocket,c->ptr->ai_addr, (int)c->ptr->ai_addrlen);
		if(rc == SOCKET_ERROR)
		{
			closesocket(c->clientSocket);
			c->clientSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}
	
	freeaddrinfo(c->result);
	
	if(c->clientSocket == INVALID_SOCKET)
	{
		cerr << "unable to connect to server" << endl;
		WSACleanup();
		exit(1);
	}
}