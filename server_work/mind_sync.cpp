/*
 * mind_sync.cpp: Multi-threaded TCP server to handle game requests from clients
 * and starting a new thread for each game.
*/

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <mutex>
#include <string>
#include <thread>
#include <set>
#include <utility>
#include <vector>
#include <boost/algorithm/string/predicate.hpp>

#include <sqlite3.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <pthread.h>

#define BUFSIZE	2048
#define	QLEN 		32
#define	LENGTH		2000
#define NUM_THREADS 2000
#define SUCCESS	"Success"

// TODO(sanisha): Make sure these are the same strings read by client.
// This would act as protocol between client and server.
#define USERNAMEERROR "USER_NAME_ERROR"
#define PASSWORDERROR	"PASSWORD_ERROR"
#define USERNAMEVALID	"USER_NAME_VALID"
#define PASSWORDVALID	"PASSWORD_VALID"
#define GAME_END "GAME_END"

#define WRITE_OUT_BUFFER if (write(sd, outBuf, strlen(outBuf)) < 0) { errexit("Error in writing to the socket\n"); }

#define WORDS_FILE_LOC "words.txt"
#define DBNAME "MindSync.db"
#define TABLENAME "Users"

using namespace std;

int errexit(const char *format, ...);
int passiveTCP(const char *service, int qlen);
int validateClientUsername(int sd, string& username);
string trim(string);

// Class for lock protected operations on socket descriptors.
// This also serves as the current state of the server.
//
class ProtectedSockets {
	private:
    std::mutex mutex;
		// Pair is (username, socket_id/file_descriptor).
		std::vector<std::pair<string, int>> fds;
		std::set<std::pair<string, int>> free_players;
	public:
		void push_back(string& username, int fd) {
			mutex.lock();
			pair<string, int> username_fd(username, fd);
			fds.push_back(username_fd);
			free_players.insert(username_fd);
			mutex.unlock();
		}	
		
		bool search_active_user(string uname) {
			mutex.lock();
			auto iter = std::find_if(fds.begin(), fds.end(), [& uname](const std::pair<string, int>&p) {return p.first == uname;});
			bool isFound = (iter != fds.end());
			mutex.unlock();
			return isFound;
		}		

		std::pair<string, int> operator [](int index) {
	    	return fds[index];	
		}

		void search_and_delete(std::pair<string, int>& player) {
			mutex.lock();
			auto iter = find(fds.begin(), fds.end(), player);
			fds.erase(iter);
			free_players.erase(player);
			mutex.unlock();
		}
	
		int size() {
			return fds.size();
		}

		int free_players_count() {
			return free_players.size();
		}

		pair<string, int> get_free_player() {
			mutex.lock();
			auto player = *(free_players.begin());
			free_players.erase(free_players.begin());
			mutex.unlock();
			return player;
		}
		
		int add_free_player(std::pair<string, int> player) {
			mutex.lock();
			free_players.insert(player);
			mutex.unlock();
		}
		
		void dump_state() {
			cout << "All players: " << endl;
			for (auto player : fds) {
				cout << player.first << ":" << player.second << endl;	
			}

			cout << "Free players: " << endl;
			for (auto player : free_players) {
				cout << player.first << ":" << player.second << endl;
			}
		}
};

// All the active connections.
ProtectedSockets active_connections;

// All the words loaded.
vector<string> words;

class User {
public:
	int id;
	string username;
	string password;
	int total_score = 0;
	int best_score = 0;
	int worst_score = 0;
	int total_games = 0;
};

// Callback function for select query
static int select_callback(void* users, int argc, char **argv, char **colName) {
	auto users_vector = (vector<User>*)users;
	User user;
	user.id = std::stoi(argv[0]);
	user.username = argv[1];
	user.password = argv[2];

	user.total_score = std::stoi(argv[3]);
	user.best_score = std::stoi(argv[4]);
	user.worst_score = std::stoi(argv[5]);
	user.total_games = std::stoi(argv[6]);
	users_vector->push_back(user);
	return 0;
}

// Class for lock protected database operations.
std::mutex db_lock;
class Database {
	private:
		Database() {}
	public:
		static bool isLoaded; 
		static sqlite3 *db;
		static vector<User>* users;

		static bool Load() {
			if (isLoaded) {
				return true;
			}

			db_lock.lock();
			int retCode = sqlite3_open(DBNAME, &db);
	
			if(retCode) {
				cout << "Error opening the database, error " <<  sqlite3_errmsg(db) << endl;
				return false;
			}

			char *errorCode = 0;
			string select_query("SELECT * FROM ");
			select_query.append(TABLENAME);
	   		retCode = sqlite3_exec(db, select_query.c_str(), select_callback, (void *)users, &errorCode);
			isLoaded = true;
			db_lock.unlock();
			return true;
		}

		static int searchUserIndex(const string& username) {
			int index = -1;
			for(int i=0; i < users->size(); ++i) {
				if ((*users)[i].username.compare(username) == 0) {
					return i;
				}	
			}
			return index;
		}
	
		static bool searchUser(const User& user) {
			for (auto u : *Database::users) {
				if (user.username.compare(u.username) == 0) {
					return true;
				}
			}	
			return false;
		}
	
		// If user exits update the record, otherwise insert the record.
		static bool AddUser(const User& user) {
			db_lock.lock();
			if (!isLoaded) {
				Database::Load();
			}

			if (Database::searchUser(user)) {
				db_lock.unlock();
				return UpdateUser(user);
			} else {
				char *insert_query_template = "INSERT INTO %s ('USERNAME', 'PASSWORD', 'TOTAL_SCORE', 'BEST_SCORE', 'WORST_SCORE', 'TOTAL_GAMES') VALUES(\"%s\", \"%s\",%d, %d, %d, %d)";
				char insert_sql[300];
				sprintf(insert_sql, insert_query_template, TABLENAME, user.username.c_str(), user.password.c_str(), user.total_score, user.best_score, user.worst_score, user.total_games); 
		
				char *errorCode = 0;
				sqlite3_exec(db, insert_sql, NULL, 0, &errorCode);
				Database::users->push_back(user);
				db_lock.unlock();
				return true;
			}
		}
		
		// Update new user stats
		static bool UpdateUser(const User& user) {
			db_lock.lock();
			for(auto& u : *users) {
				if (u.username == user.username) {
					u.password = user.password;
					u.total_score = user.total_score + u.total_score;
					u.best_score = (user.best_score > u.best_score) ? user.best_score : u.best_score;
					u.worst_score = user.worst_score < u.worst_score ? user.worst_score : u.worst_score;
					u.total_games = user.total_games + u.total_games;
					db_lock.unlock();
					return true;
				}
			}
			db_lock.unlock();
			return false;
		}

		static bool syncUserToDb(const User& user) {
			db_lock.lock();
			char *update_sql_template = "UPDATE %s SET PASSWORD = \"%s\", TOTAL_SCORE = %d, BEST_SCORE = %d, WORST_SCORE = %d, TOTAL_GAMES = %d WHERE USERNAME = \"%s\"";
			char update_sql[300];
			sprintf(update_sql, update_sql_template, TABLENAME, user.password.c_str(), user.total_score, user.best_score, user.worst_score, user.total_games, user.username.c_str());
			
			char *errorCode = 0;
			sqlite3_exec(db, update_sql, NULL, 0, &errorCode);
			db_lock.unlock();
			return true;
		}
 
		static bool syncToDb() {
			for(auto& u : *users) {	
				syncUserToDb(u);
			}	
			return true;
		}
};
bool Database::isLoaded = false;
sqlite3* Database::db = NULL;
vector<User>* Database::users = new vector<User>();


// Players who would be playing against each other. This
// struct stores username and socket descriptor.
struct GamePlayers {
	pair<string, int> player1;
	pair<string, int> player2;
};

// Function to provide current timestamp.
string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
    return buf;
}

// Function to validate the client username and password.
int validateClientUsername(int sd, string& username) {
    char inBuf[BUFSIZE + 1];
    char outBuf[BUFSIZE + 1];
    int isValid = 0;
    int recvLen;
    bool isActive = false;
    User user;
    int successFlag = 0;
    string passWord;
    string repassWord;	
    string userName;
    int msgLen;
    string msg;

    strcpy(outBuf, "Enter 1 for New User or 2 if already registered\n");
    write(sd, outBuf, strlen(outBuf));
    if ((recvLen = read(sd, inBuf, BUFSIZE)) > 0) { 
		inBuf[recvLen] = '\0';        
    }

    string input = trim(string(inBuf));
    if (input.compare("1") == 0) {
                cout << "The input:" << input << endl;
		int userFlag = 0;
		while(1) {
	    	strcpy(outBuf, "Enter the username you wish to register:\n");
	    	WRITE_OUT_BUFFER
	    	if ((recvLen = read(sd, inBuf, BUFSIZE)) > 0) { 
				inBuf[recvLen] = '\0';        
	    	}
	    	userName = trim(string(inBuf));
	    	int index = Database::searchUserIndex(userName);

			//New user and user name is not already taken
	    	if (index == -1) {
				while(1) {
		    		strcpy(outBuf, "Enter your Password:\n");
		    		WRITE_OUT_BUFFER
	
		    		if ((recvLen = read(sd, inBuf, BUFSIZE)) > 0) { 
			    		inBuf[recvLen] = '\0';        
		    		}
		    		passWord = trim(string(inBuf));
		    		strcpy(outBuf, "Renter your Password:\n");
		    		WRITE_OUT_BUFFER
		    		if ((recvLen = read(sd, inBuf, BUFSIZE)) > 0) { 
						inBuf[recvLen] = '\0';        
		    		}
		    		repassWord= trim(string(inBuf));
		    		//cout <<userName <<"hi"<< passWord <<"hi"<<repassWord; 
					if(passWord==repassWord) {
			    		userFlag = 1;
			    		break;
					}
					else {
			    		strcpy(outBuf, "Passwords don't match. Please enter it again\n");
		    	    	WRITE_OUT_BUFFER
					}
				}
	    	}
	    	else {
				strcpy(outBuf, "Username already exists. Please enter a different username.\n");
	    		WRITE_OUT_BUFFER	
	   		}
	    	if (userFlag == 1) {
				break;
			}
		} 
		user.username = userName;
		user.password = passWord;
		Database::AddUser(user);
		strcpy(outBuf, "Registration Successful!\n");
		WRITE_OUT_BUFFER
		input = "2";
    }
	// Existing user login
    if (input.compare("2") == 0) {
		cout << "Checking for username for option 2" << endl;
		strcpy(outBuf, "Enter Credentials to start the game:\n");
		WRITE_OUT_BUFFER

		// Ask for username
		while(1) {
		    isValid = 0;
	    	strcpy(outBuf, "Enter Username:\n");
	    	WRITE_OUT_BUFFER

 	    	if ((recvLen = read(sd, inBuf, BUFSIZE)) > 0) { 
	   			inBuf[recvLen] = '\0';        
				string username = trim(string(inBuf));

				int user_index = Database::searchUserIndex(username);
				//cout<<user_index << endl;
				if (user_index == -1) {
		    		strcpy(outBuf, "Not a valid UserName!\n");
					isValid = 0;
				} else {
		    		user = (*(Database::users))[user_index];
		    		isActive = active_connections.search_active_user(username);
		    		if (isActive) {
						cout << "User already logged in.\n";
						strcpy(outBuf, "User already logged in, exiting.\n");
						WRITE_OUT_BUFFER
						return isValid;
		    		}
		    		isValid = 1;
		    		strcpy(outBuf, "Enter Password:\n");
				}
				WRITE_OUT_BUFFER
			}
			break;
		}
		// Check for entered password
		if (isValid) {
			while((recvLen = read(sd, inBuf, BUFSIZE)) > 0) {
				inBuf[recvLen] = '\0';
				string password = trim(string(inBuf));
				if (password.compare(user.password) == 0) {
           		    successFlag = 1;
			    	fprintf(stdout, PASSWORDVALID);
			    	strcpy(outBuf, "Success!\n");
				} else {
 			    	fprintf(stdout, PASSWORDERROR);
			    	strcpy(outBuf, "Wrong Password!\n");
			    	isValid = 0;
				}		
				break;
		    }
		 	WRITE_OUT_BUFFER		
		}
		username.clear();
		username.append(user.username);
		return isValid;
	}
	return isValid;
}

// Function to check for valid username and password combination.
int userNameHandler(int fd) {
	fprintf(stdout, "Checking for username and password\n");
		
	// Validate username and password for three attempts.
	string username;
	// Make sure if one user is already logged in, don't let him login again.
	if (!validateClientUsername(fd, username)) {
		fprintf(stdout, "Wrong username and password or user already logged in, exiting \n");
		if (!pthread_detach(pthread_self())) {
			fprintf(stdout, "Thread detached for the given sock_id: %d \n", fd); 
		}
		close(fd);
	} else {
		active_connections.push_back(username, fd);
		fprintf(stdout, "Success! Its a match \n");
	}
	return 0;
}

// Read all the words from file.
void readWordsFile() {
	ifstream wordFile(WORDS_FILE_LOC);
	
	cout << "Reading from file started..." + currentDateTime() << endl;
	string line;
	
	while (getline(wordFile, line)) {
		words.push_back(line);
		line.clear();
	}
	
	wordFile.close();
	cout << "Reading done at: " << currentDateTime() << endl;

	if (words.empty()) {
		cerr << "No words found" << endl;
	}	
}

// Function to find random position in list.
int randomPosition(vector<int>& arr) {
  int randomNum;
  bool found = false;
  while (found == false) {
    randomNum = rand() % (words.size());
    if (find(arr.begin(), arr.end(), randomNum) == arr.end()) {
      arr.push_back(randomNum);
      found = true;
    }
  }
  return randomNum;
}

// This function takes care of a game between two selected players.
void* gameHandler(void* game_players) {
	GamePlayers gp = *((GamePlayers *)game_players);
	int fd1 = gp.player1.second;
	int fd2 = gp.player2.second;
	
	string username1 = gp.player1.first;
	string username2 = gp.player2.first;

	// Save user stats
	User user1;
	User user2;

	user1.username = username1;
	user2.username = username2;

	int game_count = 1;

	char inBuf[BUFSIZE + 1];
	int recvLen;

	int game_score = 0;

	bool new_word_wanted = true;
	string word = "";
	int word_try_count = 1;
	signal(SIGPIPE, SIG_IGN);
	vector<string> responseVal;
	bool same_response_flag=false;

	while(1) {
		// Check if both the sockets are alive/valid.
		int errorCode1 = -1, errorCode2 = -1;
		socklen_t len = sizeof (errorCode1);
		
		// TODO(sanisha): These were not functioning correctly, therefore, I 
		//                have commented them out, may explore later if we
		//                could leverage these.
		// getsockopt (fd1, SOL_SOCKET, SO_ERROR, &errorCode1, &len) << endl;
		// getsockopt (fd2, SOL_SOCKET, SO_ERROR, &errorCode2, &len) << endl;

		bool game_done = false;
		vector<int> arr;
		if (new_word_wanted) {
			word_try_count = 1;
	    	int pos = randomPosition(arr);
      		word = words[pos];
      		cout << "Sent word: " << word << endl;
		} 

		// Send the same word to both the sockets and wait for responses.
		cout << "Going to write to " << "username:fd = " << username1 << ":" << fd1 << endl;
		errno = 0;
		write(fd1, word.c_str(), word.length());
		if (errno != 0) {
			write(fd2, "GAME_END", 8);
			cout << "Notified " << "username:fd = " << username2 << ":" << fd2 << " of game shutdown." << endl;

			// Remove this player from active connections.
			active_connections.search_and_delete(gp.player1);

			// Add the other player to free players list.
			active_connections.add_free_player(gp.player2);
			break;
		}
		errno = 0;
		cout << "Going to write to " << "username:fd = " << username2 << ":" << fd2 << endl;
		write(fd2, word.c_str(), word.length());
		if (errno != 0) {
			write(fd1, "GAME_END", 8);
			cout << "Notified " << "username:fd = " << username1 << ":" << fd1 << " of game shutdown." << endl;
			
			active_connections.search_and_delete(gp.player2);
			active_connections.add_free_player(gp.player1);
			break;
		} 
	
		// Wait for 15 seconds.
		// TODO(sanisha): Tell the client to put a 10 sec wait. Server
		// need not do anything. If user does not responds back in 10
    	// sec, client should send NO_RESPONSE;
		//}
		int recvLen;
		
		cout << "Going to read from " << "username:fd = " << username1 << ":" << fd1 << endl;
		while((recvLen = read(fd1, inBuf, BUFSIZE)) > 0) {
			inBuf[recvLen] = '\0';
			break;
		}
		string response1 = trim(string(inBuf));
		cout << "Read from username:fd = " << username1 << ":" << fd1 << " response: " << response1 << endl;
		inBuf[0] = '\0';

		cout << "Going to read from " << "username:fd = " << username2 << ":" << fd2 << endl;
		while((recvLen = read(fd2, inBuf, BUFSIZE)) > 0) {
			inBuf[recvLen] = '\0';
			break;
		}
		string response2 = trim(string(inBuf));	
		cout << "Read from username:fd = " << username2 << ":" << fd2 << " response: " << response2 << endl;
		inBuf[0] = '\0';

		//compare words
		//remove white spaces for responses
		response1.erase(0, response1.find_first_not_of(' '));       //prefixing spaces
		response1.erase(response1.find_last_not_of(' ')+1);

		response2.erase(0, response2.find_first_not_of(' '));       //prefixing spaces
		response2.erase(response2.find_last_not_of(' ')+1);
	
		cout << "After erasing, re1: "<< response1 << " re2: " << response2 << endl;
		if (boost::iequals(response1,response2)) {
			int temp_flag = 0;
			// Check if it is repeated answer
			vector<string>::const_iterator iter;
			for(iter = responseVal.begin(); iter !=responseVal.end(); iter++) {
				if(response1 == *iter) {
					temp_flag = 1;		
					same_response_flag = true;
					cout << "The response is repeated, send new response" << endl;
					break;	
				}
			}
			if (temp_flag == 0) {
				cout << "Different response" << endl;
				same_response_flag = false;
				game_score += (word_try_count++ * 100);
				new_word_wanted = false;
				responseVal.push_back(response1);				
			}
		} else {
			same_response_flag = false;
			new_word_wanted = true;
			responseVal.clear();
			cout << "No match in responses" << endl;
		}

		cout << "\n*********************\n";
		cout << "Dumping state: " << endl;
		active_connections.dump_state();	
		cout << "State Dump Complete" << endl;
		cout << "\n*********************\n";

		cout << "Game score is " << game_score << endl;
		user1.total_score = game_score;
		user2. total_score = game_score;

		user1.total_games = game_count;
		user2.total_games = game_count;
		
		//TODO: We need to update user only once game ends
		//Database::UpdateUser(user1);
		//Database::UpdateUser(user2);
	}
	return NULL;
}

void* syncMemoryToDb(void*) {
	while(1) {
		sleep(30);
		cout << "##### Sinking game state to database for all users" << endl;
		Database::syncToDb();
	}
}

/*
 * startNewGames: Main function to start new games between free players.
*/
void * 
startNewGames(void* ) {
  // Read words file.
  readWordsFile();
	
	// Seed the random number.
	srand(time(0));

	while(1) {
		if (active_connections.free_players_count() >= 2) {
			pair<string, int> player1 = active_connections.get_free_player();
			// TODO(sanisha): There is a potential problem here, if active player count has decreased by the time control reaches here.
      // i.e. after check up there in if statement. But it is not a big concern so don't worry.
			pair<string, int> player2 = active_connections.get_free_player();
			
			cout << " Player1: " << player1.first << ":" << player1.second << " and Player2: " << player2.first << ":" << player2.second << endl;
			
			// Start new game for these two players.
			GamePlayers gp;
			gp.player1 = player1;
			gp.player2 = player2;

	    pthread_t th;
      if (pthread_create(&th, NULL, (void * (*) (void *))gameHandler, (void *) &gp) < 0) {
			  cout << "This thread creation failed." << endl;
      }
		}
		sleep(5);
	}
	return 0;
}

/*
 * main: Main function to get client's requests and create threads for every
 * new game.
*/
int main(int argc, char *argv[])
{
 	pthread_attr_t ta;
	pthread_t th;
	
	char *service;		/* service name or port number */
	int msock, ssock;
	int thread_num = 0, n;
	int num_players = 0;
	
	struct sockaddr_in clientAddr;
	socklen_t addr_len;

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

	// Load database.
	Database::Load();
	
	// Start a thread which syncs state to memory every 1 min.	
	if (pthread_create(&th, &ta, (void * (*) (void *))(syncMemoryToDb), NULL) < 0) {
	  errexit("Error in creating thread for starting new game.\n");
	}

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
	}
}



