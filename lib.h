#pragma once
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <string>
#include <tuple>
#include <vector>
using namespace std;


typedef struct {
	struct addrinfo *result;
	struct addrinfo *ptr;
	struct addrinfo hints;
	SOCKET clientSocket;
	SOCKET listenSocket;
	WSADATA wsaData;
} Connection_struct;








// directory listings 
tuple< vector<string>, vector<string> > listDirs(const char *dirs);
tuple< vector<string>, vector<string> > listDir(const char *dir);

