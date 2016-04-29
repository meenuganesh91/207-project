#include "PracticalSocket.h"  // For Socket, ServerSocket, and SocketException
#include <iostream>           // For cout, cerr
#include <cstdlib>            // For atoi()  
#include <pthread.h>          // For POSIX threads  
#include <map>
const int RCVBUFSIZE = 32;
const int NAMESIZE = 100;
void HandleTCPClient(TCPSocket *sock);     // TCP client handling function
void *ThreadMain(void *arg);               // Main program of a thread  

int main(int argc, char *argv[]) {
    if (argc != 2) {                 
      	cerr << "Usage: " << argv[0] << " <Server Port> " << endl;
    	exit(1);
    }
    unsigned short echoServPort = atoi(argv[1]);    // First arg:  local port  

    try {
    	TCPServerSocket servSock(echoServPort);   // Socket descriptor for server  
	for (;;) {     
	    // Create separate memory for client argument  
	    TCPSocket *clntSock = servSock.accept();
	    // Create client thread  
	    pthread_t threadID;             
	    if (pthread_create(&threadID, NULL, ThreadMain, 
	        (void *) clntSock) != 0) {
	        cerr << "Unable to create thread" << endl;
	        exit(1);
      	    }
        }
    } catch (SocketException &e) {
      	cerr << e.what() << endl;
     	exit(1);
    }
  // NOT REACHED
  return 0;
}

// TCP client handling function
void HandleTCPClient(TCPSocket *sock) {
    cout << "Handling client ";
    try {
        cout << sock->getForeignAddress() << ":";
    } catch (SocketException &e) {
    	cerr << "Unable to get foreign address" << endl;
    }
    try {
    	cout << sock->getForeignPort();
    } catch (SocketException &e) {
    	cerr << "Unable to get foreign port" << endl;
    }
  	cout << " with thread " << pthread_self() << endl;

    char echoBuffer[RCVBUFSIZE+1];
    int recvMsgSize;
    char userName[NAMESIZE + 1];
    int count=0;
    map<string,string> user_pwd_map;
    user_pwd_map["arthi"] =  "arthi";
    user_pwd_map["priya"] = "priya";
    user_pwd_map["sanisha"] = "sanisha";
    user_pwd_map["meenu"] = "meenu";
    user_pwd_map["priyanka"] = "priyanka";
    int msgLen;
    char uName[100],pWord[100],message[100];
    string msg;
    int successFlag=0;
    while(count<3 && successFlag!=1){
	
 	while ((recvMsgSize = sock->recv(echoBuffer, RCVBUFSIZE)) > 0) { 
	   echoBuffer[recvMsgSize] = '\0';        
           cout << echoBuffer << endl;        
	   int i=0,k=0;
           while(echoBuffer[i]!='~'){
	 	uName[i] = echoBuffer[i];
		i++;
	   }
	   uName[i]='\0';
	   i++;
           while(echoBuffer[i]!='\0'){
		pWord[k]=echoBuffer[i];
		i++;k++;
	   }
	   pWord[k]='\0';
	   //cout << uName << " , " << pWord << '\n';
	   msg = "Not a valid UserName!";
	   msgLen = 35;
	   for (map<string,string>::iterator it=user_pwd_map.begin(); it!=user_pwd_map.end(); ++it){
	       if(it->first== uName && it->second== pWord){
	       	   msg = "Success!";
		   msgLen = 8;
		   successFlag=1;
		   break;
		}
	       else if(it->first== uName && it->second != pWord){
		   msg = "Wrong Password!";
		   msgLen = 15;
		   break;		
		}
    	   }
	   int j=0;  
	   while(j<msg.length()){
		message[j]=msg[j];j++;
	   }
	   message[j]='\0';
	   cout << msg << " , " << message << '\n';
	   sock->send(message, msgLen);
	}
	count++;
	
    }
}

void *ThreadMain(void *clntSock) {
  // Guarantees that thread resources are deallocated upon return  
  //pthread_detach(pthread_self()); 
	//cout << pthread_self() << endl;
    int count=0;	
    if(count==3){
   	if ( ! pthread_detach(pthread_self())){
            cout << "Thread detached successfully !!!" << endl;
	    count=0;
	}

    }
    HandleTCPClient((TCPSocket *) clntSock);
    count++;
    delete (TCPSocket *) clntSock;
    return NULL;
}
