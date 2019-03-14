#pragma once
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <string>
#include <tuple>
#include <vector>
#include "lib.h"
using namespace std;




// class declaration
class Connection
{
public:
	// constructor
	Connection(Connection_struct c);
	~Connection(); // destructor
	// raw buffer send functions these will wrap the higher level data sends
	int checkedRecv(char *buffer, int flag = 0);
	void checkedSend(const char *buffer,int len, int flag = 0);
	
	// abstracted data sends
	void sendInt(const int num);
	int recvInt(void);
	void recvFile(string dir, string prefix);
	void sendFile(string filename);
	void sendString(string str);
	string recvString(void);
	long int recvLong(void);
	void sendLong(const long int);
	
	
	// overloads
	// could add (std::ostream&) and (std::istream&) to allow c << str1 << str2;
	// but it is not really worthwhile as it would only be used in one place in the code
	void operator << (string str);
	void operator >> (string &str);
	void operator << (int num);
	void operator >> (int& num);
	void operator << (long int num);
	void operator >> (long int& num);
	
	// file sends

	void send_files(string dir);
	void recv_files(string dir);	
	
private:
	Connection_struct c;
};