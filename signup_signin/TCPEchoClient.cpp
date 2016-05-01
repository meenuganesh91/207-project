#include "PracticalSocket.h"  // For Socket and SocketException
#include <iostream>           // For cerr and cout
#include <cstdlib>            // For atoi()
#include <cstring>
#include <string>
using namespace std;

const int RCVBUFSIZE = 100;   

int main(int argc, char *argv[]) {
  if ((argc < 3) || (argc > 3)) {     
    cerr << "Usage: " << argv[0] 
         << " <Server> [<Server Port>]" << endl;
    exit(1);
  }

  string servAddress = argv[1]; 
  unsigned short echoServPort = atoi(argv[2]);

  try {
    TCPSocket sock(servAddress, echoServPort);

    char pwd[200];
    string userName;
    string input;
    string passWord;
    string repassWord;
    char buf[10] = "newUser";
    int count=0;
    cout << "Enter 1 for New User or 2 if already registered" << endl;
    getline(cin, input);

//**** Handling new user ****** //


    if (input == "1"){
	int buf_size = sizeof(buf) + 1;			
//	buf[buf_size] = '\0';	
        cout << buf << endl;;
	sock.send(buf, buf_size);
	
       	char echoBuffer[RCVBUFSIZE + 1];  
        int bytesReceived = 0;            
        int totalBytesReceived = 0;       
    	
	cout << "Enter the username you wish to register" << endl;
	getline(cin, userName);
	do {
		cout << "Enter your Password:" << endl;
		getline(cin, passWord);
		cout << "Make sure the passwords match!!!" << endl << " Renter your Password:" << endl;
		getline(cin, repassWord); 
	} while (passWord != repassWord);
		
	
    	int i=0;
    	while(i<userName.length()){
    		pwd[i]=userName[i];
    	 	i++;
    	}	
        pwd[i]='~';
       	i++;
        int j=0;
        while(j<passWord.length()){
    		pwd[i]=passWord[j];
    	    	i++;j++;
        }
        pwd[i]='\0';
        //cout << pwd;
        sock.send(pwd, i+1);
	bzero(echoBuffer,sizeof(echoBuffer));
    	cout << "Received: ";             
  	if ((bytesReceived = (sock.recv(echoBuffer, RCVBUFSIZE))) <= 0) {
        	cerr << "Unable to read";
            	exit(1);
        }	
       	totalBytesReceived += bytesReceived; 
        echoBuffer[bytesReceived] = '\0';    
        cout << echoBuffer;
	cout << endl;                  
    }
//*** Handling existing user **** //
    else if (input == "2"){
	//buf="oldUser";
	//cout << buf << endl;;
	sock.send("oldUser", 10);
	int successFlag=0;
	while(count < 3 && successFlag!=1) {
		cout << "Enter Username" << endl;
		getline(cin, userName);
		cout << "Enter Password" << endl;
		getline(cin, passWord);
    		int i=0;
    		while(i<userName.length()){
    			pwd[i]=userName[i];
    	    		i++;
    		}	
        	pwd[i]='~';
       		i++;
        	int j=0;
        	while(j<passWord.length()){
    	    	pwd[i]=passWord[j];
    	    	i++;j++;
        	}
        	pwd[i]='\0';
        	//cout << pwd;
        	sock.send(pwd, i+1);
       		char echoBuffer[RCVBUFSIZE + 1];  
        	int bytesReceived = 0;            
        	int totalBytesReceived = 0;       
    		//cout << "Received: ";             
  		if ((bytesReceived = (sock.recv(echoBuffer, RCVBUFSIZE))) <= 0) {
            		cerr << "Unable to read";
            		exit(1);
        	}	
       		totalBytesReceived += bytesReceived; 
        	echoBuffer[bytesReceived] = '\0';    
		if(strcmp(echoBuffer,"Success!")==0){
				successFlag=1;
			//	cout<<"hi";
		}
        	cout << echoBuffer;                  
    		cout << endl;
		count++;
	}		
    }
    else {
		cout <<"Invalid Input" << endl;
		exit(-1);
	}
    // Destructor closes the socket

  } catch(SocketException &e) {
    cerr << e.what() << endl;
    exit(1);
  }

  return 0;
}
