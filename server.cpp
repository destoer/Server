
//ref https://docs.microsoft.com/en-us/windows/desktop/winsock/complete-server-code



// must link with Ws2_32 
#define _WIN32_WINNT 0x501 // keep windows happy and define the version we want to use

#include <iostream>
#include <string>
#include <ctime>
#include <fstream>
#include <cstdlib>
#include <winsock2.h>
#include <process.h>
#include <windows.h>
#include <ws2tcpip.h>
#include "lib.h"
#include "Connection.h"
using namespace std; // saves us writing std:: everywhere (is bad practice if you are using many non standard library's) 

void server_main(void *c_ptr);
void initServer(Connection_struct *c);

int main(void)
{
	
	Connection_struct c; // var to hold all our connection info
	int rc; // return code for error checking
	
	initServer(&c); // following this server is ready to listen for connections

	for(;;) 
	{
		cout << "Listening for a new connection\n";
		rc = listen(c.listenSocket, SOMAXCONN); // listen for a new client
		if(rc == SOCKET_ERROR)
		{
			cerr << "listen failed: " << WSAGetLastError() << endl;
			closesocket(c.listenSocket);
			WSACleanup();
			return 1;
		}
		
		// accept a client connection
		c.clientSocket = accept(c.listenSocket,NULL,NULL);
		if(c.clientSocket == INVALID_SOCKET)
		{
			cerr << "accept failed: " << WSAGetLastError() << endl;
			closesocket(c.listenSocket);
			WSACleanup();
			return 1;
		}
	
		// at this point we should start a new process for serving more clients
		// by "forking" the client handler
		Connection_struct *c_ptr = &c;
		
		// pass the sturct as a void ptr 
		_beginthread(server_main,0,static_cast<void*>(c_ptr));
	}

}

// WIN32 api is designed around C 
// so args have to be passed as a void ptr
// and then typecasted


void server_main(void *c_ptr)
{
	// cast our pointer
	Connection_struct *c_ptr2 = static_cast<Connection_struct*>(c_ptr);
	Connection_struct c_tmp;

	// memcpy it as we dont want to modify the main struct or
	// every thread would be using the same connection struct and we would have problems
	memcpy(&c_tmp,c_ptr2,sizeof(Connection_struct));
	Connection c{c_tmp}; // init our class takes struct as arg ( {} is user for initialization in c++

	try
	{	
		// read in  the username and password
	
		cout << "Waiting to receive username and password\n";
	
		string password;
		string username;
	
		// "c >> " receives data and "c <<"  receives data
		c >> username; 
		c >> password;
	
		cout << username << ":" << password  << "\n";
	
		// strings to read username and pass out of the file
		string pass;
		string name;
		int id; // access right id (permissions level)
		bool passed = false;	// bool to say if our user has supplied valid credentials
	
	
		// should open the login file and compare by here
		ifstream fp{"cred.txt", ios::in};
		
		if(!fp)
		{
			cerr << "Failed to open the credentials file" << endl;
			c << -1; // indicate auth failure
			goto error;
		}
		
		// read thru the file and get filename and password and compare
		while(fp >> name >> pass >> id)
		{		
			if(pass == password && name == username)
			{
				cout << "Auth sucess" << endl;
				passed = true;
				break;
			}
		} 
		fp.close();
		
		if(passed == false)
		{
			// indicate an authentication failure
			cout << "Auth failure" << endl;
			c << -1;
			goto error;
		}
		
		c << 1; // indicate auth success 

	
		// send back the group id 
		c << id;

		int rc = 1;
		bool quit = false;
		time_t time;
		// command processing loop for a normal user 

		if(id == 1)
		{
			while(!quit)
			{
				cout << "Receiving new choice\n";
				string choice;
				c >> choice;
				
				cout << "Received choice: " << choice << "\n";
				
				
				if(choice == "Logout")
				{
					cout << "Logout request received listening for files and timestamp...\n";
					
					// recv the timestamp
					c >> time;
					
					cout << time << '\n';
					
					
					// recv the files
					c.recv_files("out/"+ name + "/");
					
					
					quit = true; // quit out of the server
					cout << "Done receiving files!\n";
				}
				
				
				else if(choice == "Get time")
				{
					c >> time;
					
					fstream fpt{name + "/user.dat", ios::in | ios::out | ios::binary};
					// recv current login time and subtract from
					// the global one					
					time_t usertime = 0;
					fpt >> usertime;

					
					usertime -=  time;
		
					c << usertime;
					
					cout << "Sending current user time remaining...\n";

				}
			}
			
			// <--- update the time logging 
			
			
			fstream fpt{name + "/user.dat", ios::in | ios::out | ios::binary};
			time_t usertime = 0;
			fpt >> usertime;
			
			cout << "Current usertime remaining: " << usertime << '\n';
			cout << "Time spent on session: " << time << '\n';
			usertime -=  time;
			
			cout << "Current time left now: " << usertime << '\n';
			
			// hit eof have to reset the seek and put pointers
			// cause apperently C++ can fail writes on a file write pointer
			// at the start of the file when the read one hits eof
			// even when we are only using the write ptr...
			
			fpt.seekp(0, ios::beg);
			fpt.seekg(0, ios::beg);
			
			fpt << usertime;
			
			if( !fpt.good() ) // log file errors
			{
				cerr << "Error: " << strerror(errno) << fp.bad() << fp.fail() << fp.eof() << endl;
			}
		}
		
		// moderator
		else if(id == 0)
		{

			while(!quit)
			{
				cout << "Receiving new choice\n";
				string choice;
				c >> choice;

				if(choice == "Download") // Send back the users appropriate files
				{
					cout << "[DEBUG] listening for username to download files for\n";
					string userdir {c.recvString()};
					
					// our file send code will handle an invalid user but it will disconnect the client
					
					c.send_files("D:/coursework/work/"+ userdir + "/");
					
				}
				
				else if(choice == "Quit") // user request a logout terminate the handler
				{
					quit  = true;
				}
				
				
				else
				{
					cout << "Invalid choice"  << endl;
				}
				
			}
		}
		
		
		else // handle a invalid permissions error
		{
			cout << "Unknown permission level detected" << id << endl;
			goto error;
		}
		
		cout << "Closing client server successfully" << endl;
			
		
	} 
	
	catch(exception &e) // failure to send data etc...
	{
		cout << "Exception detected closing client handler\n";
		_endthread(); // end the thread handling the current client
	}

	
	
	
	
	cout << "Client handler terminated successfully!" << endl;
	_endthread();
	return;
error: // Error handling 
	cout << "Error handling client, closing thread...";
	_endthread();
	return;
	
}


// init the server ready to listen for a client connection
void initServer(Connection_struct *c)
{
	int rc = WSAStartup(MAKEWORD(2,2), &c->wsaData);
	if(rc != 0) // non zero is a fatal error
	{
		cerr << "WSAStarted failed:" << rc << endl;
		exit(1); // fine to exit here instead of raising an exception
				// because the server will be in a useless state if the server
				// cant take incoming connections
	}
	
	ZeroMemory(&c->hints,sizeof(c->hints));
	c->hints.ai_family = AF_INET;
	c->hints.ai_socktype = SOCK_STREAM;
	c->hints.ai_protocol = IPPROTO_TCP;
	c->hints.ai_flags = AI_PASSIVE;
	
	// resolve the server address and port
	// hardcoded port
	rc = getaddrinfo(NULL,"6996", &(c->hints), &(c->result));
	if(rc != 0)
	{
		cerr << "getaddrinfo failed: " << rc << endl;
		WSACleanup();
		exit(1);
	}
	
	// create a socket connecting to the server
	c->listenSocket = socket(c->result->ai_family, c->result->ai_socktype, c->result->ai_protocol);
	if(c->listenSocket == INVALID_SOCKET)
	{
		cerr << "socket failed: " << WSAGetLastError() << endl;
		freeaddrinfo(c->result);
		WSACleanup();
		exit(1);
	}
	
	
	// bind the socket
	rc = bind(c->listenSocket,c->result->ai_addr, (int)c->result->ai_addrlen);
	if(rc == SOCKET_ERROR)
	{
		cerr << "bind failed: " << WSAGetLastError() << endl;
		freeaddrinfo(c->result);
		closesocket(c->listenSocket);
		WSACleanup();
		exit(1);
	}
	
	freeaddrinfo(c->result);
}