/*
 * mind_sync_server.cpp: This file contains all the important server functions
 * required by the main server.
*/
#include <algorithm>
#include <iostream>
#include <locale>
#include <string>
#include <vector>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
		
#define BUFSIZE	2048
#define NAMELEN	100

int errexit(const char *format, ...);
using namespace std;

/*
 * trim: Function to remove random characters from input username and password. 
*/
string 
trim(string str) {
	string ans;
	for (auto c : str) {
		if (isalnum(c)) {
		  ans.push_back(c);
		} else {
			break;
		}
	}
	return ans;
}
