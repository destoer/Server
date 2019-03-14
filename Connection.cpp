#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <string>
#include <tuple>
#include <vector>
#include <iostream>
#include <fstream>
#include "Connection.h"
using namespace std;


// constructor
Connection::Connection(Connection_struct con) :c(con) {}

// destructor
Connection::~Connection()
{
	closesocket(c.clientSocket);
}

// recv from the client for a fixed length
// check len does not exceed max send size
int Connection::checkedRecv(char *buffer, int flag)
{
	int len; // could type pun the data with a union but this is cleaner (and c++ does not play well with it)
	// recv the length of sent data
	int rc = recv(c.clientSocket,(char*)buffer,sizeof(int),flag);
	if( rc < 0 )
	{
		cerr << "recv len failed : " << WSAGetLastError() << endl;
		//closesocket(c.clientSocket);
		//WSACleanup();
		throw runtime_error("recv len failed" + WSAGetLastError());
		return -1;
	}
	
	memcpy(&len,buffer,sizeof(int)); // get int out of our buffer
	
	// should validate not over maxlen here too
	if(len < 0)
	{
		cerr << "error length out of range " << endl;
		//closesocket(c.clientSocket);
		//WSACleanup(); // used to call this to clean up data but needs to be called at the end as it will 
						// clean up the state for the entire program and not just the thread
		throw runtime_error("error length out of range");
		return -1;
	}
	
	
	// recv data for length
	rc = recv(c.clientSocket,(char*)buffer,len,flag);
	
	if( rc < 0 )
	{
		cerr << "recv failed: " << WSAGetLastError() << endl;
		//closesocket(c.clientSocket);
		//WSACleanup();
		throw runtime_error("recv failed" + WSAGetLastError());
		return -1;
	}	
	
	return rc;
}



void Connection::checkedSend(const char *buffer,int len, int flag)
{
	// validate length here
	if(buffer == NULL && len > 0)
	{
		cout << "attempted to send " << len << "bytes"
			<< " of a null buffer";
		throw runtime_error("Attempted to send a empty buffer");
		return;
	}
	
	// send the length of the message
	int rc = send(c.clientSocket,(char*)&len,sizeof(int),flag);
	if(rc == SOCKET_ERROR)
	{
		cerr << "send len failed: " << WSAGetLastError() << endl;
		//closesocket(c.clientSocket);
		//WSACleanup();
		throw runtime_error("send len failed" + WSAGetLastError());
		return;
	}	
	
	// send the actual buffer
	rc = send(c.clientSocket,(char*)buffer,len,flag);
	if(rc == SOCKET_ERROR)
	{
		cerr << "send failed: " << WSAGetLastError() << endl;
		//closesocket(c.clientSocket);
		//WSACleanup();
		throw runtime_error("Send failed: " + WSAGetLastError());
		return;
	}
}


void Connection::sendInt(const int num)
{
	char buffer[sizeof(int)] = {0};
	memcpy(buffer,&num,sizeof(int));
	
	checkedSend((char*)buffer,sizeof(int));
}

int Connection::recvInt(void)
{
	char buffer[sizeof(int)] = {0};
	int num = 0;
	int rc = checkedRecv((char*)buffer);
	if(rc < 0)
	{
		cout <<"[RECVINT] failed with " << rc << endl;
		//closesocket(c.clientSocket);
		//WSACleanup();
		throw runtime_error("[RECVINT] failed with %d" + rc);
		return -1;
	}	
	memcpy(&num,buffer,sizeof(int));
	return num;
}

void Connection::sendLong(const long int num)
{
	// basically the same as int just storing the Long as 
	// an array of bytes as thats how the data has to be sent
	// could also type pun it with a union but this is more standard
	char buffer[sizeof(long int)] = {0};
	memcpy(buffer,&num,sizeof(int));
	
	checkedSend((char*)buffer,sizeof(long int));
}

long Connection::recvLong(void)
{
	char buffer[sizeof(long int)] = {0};
	int num = 0;
	int rc = checkedRecv((char*)buffer);
	if(rc < 0)
	{
		cout <<"[RECVINT] failed with " << rc << endl;
		//closesocket(c.clientSocket);
		//WSACleanup();
		throw runtime_error("[RECVINT] failed with %d" + rc);
		return -1;
	}	
	memcpy(&num,buffer,sizeof(long int));
	return num;
}



// have to make sure the data send is done before we keep blasting packets
// or it might not be ready
void Connection::recvFile(string dir, string prefix)
{
	char buffer[512] = {0}; 
	int len = 1;				

	string filename = recvString();
	cout << "receiving file: " << filename << "...";
	
	
	int i;
	for(i = 0; i < prefix.length(); i++)
	{
		if(prefix[i] != filename[i])
		{
			break;
		}
	}
	
	
	
	
	filename = filename.substr(i,filename.length());
	cout << "final path: " << filename << "...";
	ofstream fp{dir + filename, ios::out | ios::binary};
	int rc = 1;
	while(rc == 1)
	{
		rc = recvInt();
		if(rc == -2) { break;  }
		len = checkedRecv(buffer);
		fp.write(buffer,len);
	}
	
	sendInt(5);
	
	fp.close();
	cout << "done\n";
}


void Connection::sendFile(string filename)
{
	char buffer[512] = {0};
	//checkedSend(filename,strlen(filename)); // send the filename
	sendString(filename);
	cout << "Sending file: " << filename;
	

	FILE *fp = fopen(filename.c_str(),"rb");
	
	if(fp == NULL)
	{
		cout << " Error opening file " << filename << "\n";
		return; // add handling
	}
	
	
	fseek(fp,0,SEEK_END);
    int fileSize = ftell(fp); // check return size
	fseek(fp,0,SEEK_SET);
	cout << " " << fileSize << " fileSize (Bytes)..."; 
	
	
	if(fileSize == 0)
	{
		// indicate EOF
		sendInt(-2);
		fclose(fp);
		cout << "done file empty\n";		
		return;
	}
	int rem = fileSize % 512;

	
	if(fileSize > 512)
	{
		for(int i{0}; i < fileSize / 512; i++)
		{
			sendInt(1);
			fread(buffer,1,512,fp);
			checkedSend(buffer,512);
		}
		
		if(rem > 0)
		{
			sendInt(1);
			fread(buffer,1,rem,fp);
			checkedSend(buffer,rem);
		}
	}
	
	else if(fileSize > 0)
	{
		sendInt(1);
		fread(buffer,1,fileSize,fp);
		checkedSend(buffer,fileSize);
	}
	
	// indicate EOF
	sendInt(-2);
	
	// wait for server to say that the file send went fine
	int rc = recvInt();
	
	if(rc != 5)
	{
		throw runtime_error{"bad file send!"};
	}
	
	fclose(fp);
	cout << "done\n";
}

void Connection::sendString(string str)
{
	checkedSend(str.c_str(),str.length());
}

string Connection::recvString(void)
{
	char buffer[512] = {0};
	checkedRecv(buffer);
	return string(buffer);
}


// overloaded functions for stream operators
// c++ will use function overloading and automatically call the
// right version of this depending on the type passed
// these are just wrapper for the internal class functions
// that are more convenient to use as we dont need to call 
// a specific function for our data 

// string recv and send

void Connection::operator << (string str)
{
	sendString(str);
}

void Connection::operator >> (string& str)
{
	str = recvString();
}

// int recv and send

void Connection::operator << (int num)
{
	sendInt(num);
}

void Connection::operator >> (int& num)
{
	num = recvInt();
}

// long int recv and send

void Connection::operator << (long num)
{
	sendLong(num);
}

void Connection::operator >> (long& num)
{
	num = recvLong();
}



void Connection::send_files(string dir)
{
	// list the files 
	vector<string> dirList;
	vector<string> fileList;
	tie(dirList, fileList) =  listDirs(dir.c_str()); 
	
	// send the directorys so the server can make a copy under /test2
	for(int i = 0; i < dirList.size(); i++)
	{
		sendString(dirList[i]);
	}
	
	
	checkedSend(NULL,0);	
	cout << "Sending over contents of files...\n";
	

	sendString(dir);
					
	// now send off all the files
	for(int i = 0; i < fileList.size(); i++)
	{
		sendInt(1);
		sendFile(fileList[i]);
	}

	sendInt(6);

	cout << "File send done\n";
	if(recvInt() != 7)
	{
		cout << "Server did not acknowledge file send\n";
	}	
}


void Connection::recv_files(string dir)
{
	// recv the files
	cout << "receiving files...\n";
	int rc = 1;
	char buffer[512] = {0};
	
	while(rc != 0)
	{
		memset(buffer,0,512); // zero mem incase the string is shorter than before
		rc = checkedRecv(buffer);
		string s2{buffer};
		string filename = dir;
		filename += s2;	
		CreateDirectory(filename.c_str(),NULL);
		cout << "Creating directory: " << filename << endl;
	}
	
	cout << "Finished receiving directory information\n";
	
	rc = 1;	

	string prefix = recvString();	
	cout << "prefix: " << prefix << "\n";
					
	while(rc != 6) // check we are not done sender will terminate when a 6 is sent
	{
		rc = recvInt();
		if(rc == 6) {  break; }
		recvFile(dir,prefix);
	}
					
					
	// indicate acknowledgment
	sendInt(7);
}