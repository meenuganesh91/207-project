/*
 * mindSync.cpp: Multhi-threaded TCP server to handle game requests from clients
 * and starting a new thread for each game.
*/

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

#define	QLEN 		32
#define	LENGHT		2000
#define NUM_THREADS 2000

struct thread_data{
   int  thread_num;
   int 	sock_id;
};

struct thread_data thread_data_array[NUM_THREADS];

int errexit(const char *format, ...);
int passiveTCP(const char *service, int qlen);
void *handler_funct(void *data);

/*
 * main: Main function to get client's requests and create threads for every
 * new game.
*/
int main(int argc,char *argv[])
{
	pthread_t p_thread[NUM_THREADS];
 	pthread_attr_t ta;
	
	char *service;		/* service name or port number */
	struct sockaddr_in clientAddr;
	int msock, ssock;
	socklen_t addr_len;
	int thread_num = 0, n;
	int num_players = 0;
	
	switch (argc) {
		case 2:
			service = argv[1];
			break;
		default:
			errexit("Error in setting server: usage: mind_sync [port] \n");
	}

	fprintf(stdout, "**** Starting server at port: %s ****\n", service);

	/* Create listening socket */
	msock = passiveTCP(service, QLEN);

	(void) pthread_attr_init(&ta);
	(void) pthread_attr_setdetachstate(&ta, PTHREAD_CREATE_DETACHED); 	

	/* Loop to get new connections */
	while(1)
	{		
		//accepting new request from client
		addr_len = sizeof(clientAddr);
		ssock = accept(msock, (struct sockaddr *) &clientAddr, &addr_len);	
	
		if (ssock < 0) {
			if (errno == EINTR)
				continue;
			errexit("Failure in accepting connections \n");
	 	}
		
		num_players++;

		if (num_players == 2 && thread_num < NUM_THREADS) {
			fprintf(stdout, "\nGame number %d starting for 2 clients\n", thread_num);
			
			num_players = 0;
			
			//storing data to structure, which will be passing to new thread
			thread_data_array[thread_num].thread_num = thread_num;
	 		thread_data_array[thread_num].sock_id = ssock;

			//creating a new thread
			n = pthread_create(&p_thread[thread_num], NULL, handler_funct,
				 (void*) &thread_data_array[thread_num]);
	
		 	if (n < 0) {
				errexit("Error in creating thread in main server \n");
			}

			thread_num++;
		}
	}
}

/*
 * handler_funct: Handler function for handling each game request.
*/
void *handler_funct(void *data)
{
    fprintf(stdout, "Thread function called for game.\n");
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


