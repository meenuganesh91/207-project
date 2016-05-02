/* Author: Sanisha Rehan
 * This file contains the code for SQLite3 connectivity using cpp.
 *
*/
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

#include <iostream>
#include <mutex>
#include <string>
#include <sstream>
#include <vector>

#define DBNAME "MindSync.db"
#define TABLENAME "Users"

using namespace std;

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

		static User* searchUser(const string& username) {
			for (auto u : *Database::users) {
				if (u.username.compare(username) == 0) {
					return &u;
				}
			}	
			return NULL;
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
		
		static bool UpdateUser(const User& user) {
			db_lock.lock();
			for(auto& u : *users) {
				if (u.username == user.username) {
					u.password = user.password;
					u.total_score = user.total_score;
					u.best_score = user.best_score;
					u.worst_score = user.worst_score;
					u.total_games = user.total_games;
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


int main() {
	Database::Load();
	User new_user;
	new_user.username = "bablu3";
	new_user.password = "taplu";
	cout<< Database::AddUser(new_user);
	
	new_user.total_score = 100;
	cout << Database::UpdateUser(new_user);
	cout << Database::syncToDb();
	return 0;
}
