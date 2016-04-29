/* Multi threaded sever*/
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <fstream>
#include <strings.h>
#include <stdlib.h>
#include <string>
#include <pthread.h>
using namespace std;


#define	QUEUE   32
#define	LENGHT	2000
#define NUM_THREADS 2000

int num_player=0;

struct thread_data{
   int  thread_id;
   int 	sock_id;
};

struct thread_data thread_data_array[NUM_THREADS];

void *handler_funct(void *data);

int main(int argc,char *argv[])
{
	int serversocket,slaveSocket;
	struct sockaddr_in serverAddr,clientAddr;
	socklen_t addr_len;
	int enable = 1,t=0,n,portnum;
	int att_int;
	pthread_t p_thread[NUM_THREADS];
 	 
	//port num as argument
	portnum = atoi(argv[1]);

	//creating socket
	if((serversocket = socket(PF_INET, SOCK_STREAM, 0))<0)
	{perror("there is an error while creating server socket\n");return 0;}

	//setting option for reuseing address
	if (setsockopt(serversocket, SOL_SOCKET, SO_REUSEADDR,&enable, sizeof(enable)) < 0) 
	{perror("Failed to set sock option SO_REUSEADDR");}

	//settting address endpoint structure
	serverAddr.sin_family = AF_INET;	
	serverAddr.sin_addr.s_addr =INADDR_ANY;
	serverAddr.sin_port = htons(portnum);

	//binding socket	
	if((bind(serversocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)))<0)
	{perror("ERROR while binding server socket");return 0;}

	//setting socket in listen mode
	if((listen(serversocket,QUEUE))<0)
	{perror("ERROR while listening server socket");return 0;}


	addr_len = sizeof(clientAddr);

	
	while(1)
	{		
		//accepting new request from client
		slaveSocket = accept(serversocket, (struct sockaddr *) &clientAddr, &addr_len);	
	
		if(slaveSocket<0)
		{
		perror("ERROR while accepting in server socket");
		continue;
	 	}
		
		num_player++;
		

		if (num_player == 2 && t < NUM_THREADS)
		{
		
		num_player=0;

	
		
		//storing data to structure, which will be passing to new thread
		thread_data_array[t].thread_id = t;
	 	thread_data_array[t].sock_id =slaveSocket;

		//creating a new thread
		n=pthread_create(&p_thread[t],NULL,handler_funct,(void*)&thread_data_array[t]);

	 	if(n!=0)
		{
		perror("ERROR while creating thread in server socket");
		continue;
		}
	
		t++;


		}
			

	}




	

	//thread join
	for(int i = 0; i <NUM_THREADS; i++)
    	{
        	pthread_join(p_thread[i], NULL);
    	}
    


	return 0;
	
}


void *handler_funct(void *data)
{
    
	struct thread_data *my_data;
	my_data = (struct thread_data *) data;
	char buffer[LENGHT];
	int sock;
	
	sock = my_data->sock_id;


	printf("game is started\n");
	fflush(stdout);

	close(sock);

    return 0;
} 


