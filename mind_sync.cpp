/*
 * mindSync.cpp: Multi-threaded TCP server to handle game requests from clients
 * and starting a new thread for each game.
*/

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <mutex>
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

#include <cstdlib>
#include <vector>
#include <unordered_set>
#include <algorithm>
using namespace std;

#define	QLEN 		32
#define	LENGTH		2000
#define NUM_THREADS 2000

struct thread_data{
   int  thread_num;
   int 	sock_id;
};

struct thread_data thread_data_array[NUM_THREADS];

int errexit(const char *format, ...);
int passiveTCP(const char *service, int qlen);
int userNameHandler(int fd);
void *handler_funct(void *data);
int validateClientUsername(int sd);
int gameHandler(int sd1, int sd2);
void readWordsFile();

// Defining class for lock protected operations on socket descriptors.
class ProtectedSockets {
	private:
    std::mutex mutex;
		std::vector<int> fds;
		std::unordered_set<int> free_players;
	public:
		void push_back(int fd) {
			mutex.lock();
			fds.push_back(fd);
			free_players.insert(fd);
			mutex.unlock();
		}	
		
		int operator [](int index) {
	    return fds[index];	
		}

		void search_and_delete(int fd) {
			mutex.lock();
			auto iter = find(fds.begin(), fds.end(), fd);
			fds.erase(iter);
			free_players.erase(fd);
			mutex.unlock();
		}
	
		int size() {
			return fds.size();
		}

		int free_players_count() {
			return free_players.size();
		}

		int get_free_player_fd() {
			mutex.lock();
			if (free_players.size() < 1) {
				return -1;
			}
			
			int fd = *(free_players.begin());
			free_players.erase(free_players.begin());
			mutex.unlock();
			return fd;
		}
};

ProtectedSockets active_connections;

/*
 * startNewGames: Main function to start new games between free players.
*/
void * 
startNewGames(void* ) {
	while(1) {
		sleep(2);
		int status;
		
		if (active_connections.free_players_count() >= 2) {
			int fd1 = active_connections.get_free_player_fd();
			int fd2 = active_connections.get_free_player_fd();
			
			cout << "File descriptor 1: " << fd1 << " and file descriptor 2: " << fd2 << endl;
			// add the two socket descriptors and start the game between them
			status = gameHandler(fd1, fd2);
		}	
	}
	return 0;
}

/*
 * main: Main function to get client's requests and create threads for every
 * new game.
*/
int main(int argc,char *argv[])
{
	pthread_t p_thread[NUM_THREADS];
 	pthread_attr_t ta;
	pthread_t th;
	
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
	readWordsFile();

	/* Create listening socket */
	msock = passiveTCP(service, QLEN);

	(void) pthread_attr_init(&ta);
	(void) pthread_attr_setdetachstate(&ta, PTHREAD_CREATE_DETACHED); 	

	// Start thread which looks at the state of number
	// of active connections. If active connections are 
  	// more than, then it spawns out a new thread which
  	// runs a game.
	if (pthread_create(&th, &ta, (void * (*) (void *))(startNewGames), NULL) < 0) {
	  errexit("Error in creating thread for starting new game.\n");
	}

	/* Loop to get new connections */
	while(1)
	{		
		// Accepting new request from client.
		addr_len = sizeof(clientAddr);
		ssock = accept(msock, (struct sockaddr *) &clientAddr, &addr_len);	
	
		if (ssock < 0) {
			if (errno == EINTR)
				continue;
			errexit("Failure in accepting connections \n");
	 	}
		
		num_players++;
		
		/* Validate username and passwords */
		if (pthread_create(&th, &ta, (void * (*) (void *))userNameHandler, (void *) ssock) < 0) {
			errexit("Error in creating thread for username validation\n");
		}
		
		/* TODO: Thread should return if username is correct or not */
		/*
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
		*/
	}
}


/*
 * userNameHandler: Function to check for valid username and password combination.
 */
int
userNameHandler(int fd) {
	fprintf(stdout, "Checking for username and password\n");
	
	/* Validate username and password for three attempts */
	if (!validateClientUsername(fd)) {
		fprintf(stdout, "Wrong username and password for 3 attempts, exiting \n");
		if (!pthread_detach(pthread_self())) {
			fprintf(stdout, "Thread detached for the given sock_id: %d after 3 attempts \n", fd); 
		}
		close(fd);
	} else {
		active_connections.push_back(fd);
		fprintf(stdout, "Success! Its a match \n");
	}
	return 0;
}

/*
 * handler_funct: Handler function for handling each game request.
*/
void *handler_funct(void *data)
{
    fprintf(stdout, "Thread function called for game.\n");
	struct thread_data *my_data;
	my_data = (struct thread_data *) data;
	char buffer[LENGTH];
	int sock;
	
	sock = my_data->sock_id;

	printf("game is started\n");
	fflush(stdout);

	close(sock);

   return 0;
} 


