
#define _WIN32_WINNT 0x501 // keep windows happy and define the version we want to use

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <string>
#include <vector>
#include <tuple>
#include <strsafe.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "lib.h"
#include "Connection.h"
#define ERROR_NO_MORE_FILES 18L


// Awful code 

tuple< vector<string>, vector<string> > listDirs(const char *dirs) // <-- recurses the dirs
{
	// list the files 
	vector<string> dirList; // the dir list 
	vector<string> fileList; // the file list 
	vector<string> dir_new; // dir list of current dir 
	vector<string> file_new; // file list of current dir 
	tie(dirList, fileList) =  listDir(dirs); // get the intial
	vector<vector<string>> dir_recur; // the list of dirs we need to recurse through
	vector<string> dir_list = dirList; // the current set of directory's we are passing
	
	// add our initial dir to our initially found directory
	for(auto j = 0; j < dir_list.size(); j++) // and to the new director
	{
		dir_list[j] = string(dirs) + dir_list[j] + "/"; 
	}
	
	for(auto j = 0; j < fileList.size(); j++) // add the dir to the file name
	{
		fileList[j] = string(dirs) + fileList[j];
	}	
	
	
	bool fin = false;
	while(!fin)
	{
		
		
		// run through the current vector
		for(auto i = 0; i != dir_list.size(); i++)
		{
			tie(dir_new, file_new) =  listDir(dir_list[i].c_str());
			if(file_new.size() > 0)
			{
				for(auto j = 0; j < file_new.size(); j++) // add the dir to the file name
				{
					file_new[j] = dir_list[i] +  file_new[j];
				}
			}
			
			if(dir_new.size() > 0)
			{
				cout << dir_new[0];
				for(auto j = 0; j < dir_new.size(); j++) // and to the new directory list
				{
					dir_new[j] = dir_list[i] + dir_new[j] + "/";
				}
			}	
					
			//dirList += dir_new; // cant do this in c++ but this is effectively what its doing
			dirList.insert( dirList.end(),dir_new.begin(),dir_new.end()); 
			//dir_recur += dir_new;
			dir_recur.push_back(dir_new); // add our new list of dirs to recurse through
				
			// fileList += file_new;
			fileList.insert(fileList.end(),file_new.begin(),file_new.end());
			
		}
 		
		if(dir_recur.size() != 0)
		{
			// get element and remove it ( i.e get our next dir to parse are remove it from list )
			dir_list = dir_recur.back();
			dir_recur.pop_back();
		}
		
		else
		{
			fin = true;
		}	
	}
	
	
	
	return make_tuple(dirList, fileList);	
}	


//https://docs.microsoft.com/en-us/windows/desktop/fileio/listing-the-files-in-a-directory
tuple< vector<string>, vector<string> > listDir(const char *dir) // <-- need to make it recurse thru dirs 
{
	vector<string> fileList;
	vector<string> dirList;
	
	
	WIN32_FIND_DATA ffd;
	LARGE_INTEGER filesize;
	char szDir[MAX_PATH];
	size_t length_of_arg;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	unsigned int dwError = 0;
	
	if(dir == NULL)
	{
		cout << "please supply a diretory name";
		exit(1);
	}
	
	// check input is path + 3 is not longer than  MAX_PATH
	// 3 extra chars are for "\*", 0
	
	StringCchLength(dir,MAX_PATH,&length_of_arg);
	
	if(length_of_arg > (MAX_PATH - 3))
	{
		cout << "\nDirectory path is too long.\n";
		exit(1);
	}
	
	
	cout << "Target directory is: " << dir << "...";
	
	// prepare a string for use with findfile functions
	// first copy the string to a buffer and append '\*' to the dir name
	
	StringCchCopy(szDir,MAX_PATH, dir);
	StringCchCat(szDir,MAX_PATH, TEXT("\\*"));
	
	// find first file in dir 
	
	hFind = FindFirstFile(szDir, &ffd);
	
	if(INVALID_HANDLE_VALUE == hFind)
	{
		cout << "error in listing directory\n";
		return make_tuple(dirList, fileList);	// make this fault tolerant 
	}
	
	// list all files in dir with info about them 
	do
	{
		if(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// ingore current dir and prev dirs or we will recurse forever
			if( (0 != strcmp(ffd.cFileName,"..")) && (0 != strcmp(ffd.cFileName,".")) )
			{
				dirList.push_back(ffd.cFileName);
			}
		}
	
		else
		{
			fileList.push_back(ffd.cFileName);
		}
	}
	while(FindNextFile(hFind, &ffd) != 0);
	
	dwError = GetLastError();
	if(dwError != ERROR_NO_MORE_FILES)
	{
		cout << "Error in listing directory2\n";
	}
	FindClose(hFind);
	cout << "done\n";
	return make_tuple(dirList, fileList);
}