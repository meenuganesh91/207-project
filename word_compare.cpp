#include <iostream>
#include <cstdlib>
#include <string> 
using namespace std;

int word_compare(string response1,string response2);


int word_compare(string response1,string response2)
{
	//remove white spaces for response1
	response1.erase(0, response1.find_first_not_of(' '));       //prefixing spaces
	response1.erase(response1.find_last_not_of(' ')+1);
	
	
	for(unsigned int i = 0; i < response1.length(); i++)
	{
		response1[i] = toupper(response1[i]);
	}

	//remove white spaces for response1
	response2.erase(0, response2.find_first_not_of(' '));       //prefixing spaces
	response2.erase(response2.find_last_not_of(' ')+1);
	
	
	for(unsigned int i = 0; i < response2.length(); i++)
	{
		response2[i] = toupper(response2[i]);
	}
	
	if (response1==response2)
		
		return 100;
	else 
		return 0;
	
	
}
