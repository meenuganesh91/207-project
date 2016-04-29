/* Author: Sanisha Rehan
 * This file contains the code for SQLite3 connectivity using cpp.
 *
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

#define DBNAME "MindSync.db"
#define TABLENAME "MindSyncUsers"

struct user_info{
	int id;
	char username[100];
	char password[100];
	char total_scores[100];
	char best_score[100];
	char worst_score[100];
	char total_games[100];
};

char *select_sql =  "SELECT * FROM %s WHERE %s";
char *insert_sql = "INSERT INTO %s (user_name, password) VALUES (\"%s\", \"%s\")";
char *update_sql = "UPDATE %s set total_scores = %d, best_score = %d, worst_score = %d, total_games = %d WHERE %s";

static int callBack(void *u_info, int argc, char **argv, char **colName) {
	int i;
	struct user_info *uinfo = (struct user_info *)u_info;
	for(i=0; i < argc; i++){
    	  printf("%s = %s\n", colName[i], argv[i] ? argv[i] : "NULL");
   	}
	strcpy(uinfo->username, (argv[1] ? argv[1] : "NULL"));
	strcpy(uinfo->password , (argv[2] ? argv[2] : "NULL"));
	strcpy(uinfo->total_scores, (argv[3] ? argv[3] : "NULL"));
	strcpy(uinfo->best_score, (argv[4] ? argv[4] : "NULL"));
	strcpy(uinfo->worst_score, (argv[5] ? argv[5] : "NULL"));
	strcpy(uinfo->total_games, (argv[6] ? argv[6] : "NULL"));	
   	printf("\n");
   	return 0;
}

int main(int argc, char *argv[])
{
	sqlite3 *db;
	char *errorCode = 0;
	int retCode;

	char *opCode = "select";		/* DB Operation desired */
	char sqlQuery[200];
	char *data = "Callback function called";
	char where_clause[200];

	struct user_info uinfo;
	switch (argc) {
		case 8:
			strcpy(uinfo.total_scores, argv[4]);
			strcpy(uinfo.best_score, argv[5]);
			strcpy(uinfo.worst_score, argv[6]);
			strcpy(uinfo.total_games, argv[7]);
		case 4:
			strcpy(uinfo.password, argv[3]);
			strcpy(uinfo.username, argv[2]);
		case 2:
			opCode = argv[1];
			break;
		default:
			fprintf(stdout, "The default command selecting as SELECT \n");
	}
	
	/* Connect to database */
	retCode = sqlite3_open(DBNAME, &db);
	
	if(retCode) {
		fprintf(stdout, "Error opening the database, error %s\n", sqlite3_errmsg(db));
		return 1;
	} else {
		fprintf(stdout, "Opened the database\n");
	}

	fprintf(stdout, "The passed param is %s \n", opCode);

	/* Select functions based on OpCode */
	if (strcmp(opCode,"select") == 0) {
		sprintf(sqlQuery, select_sql, TABLENAME, " 1=1 ");
		retCode = sqlite3_exec(db, sqlQuery, callBack, (void *)&uinfo, &errorCode);
	} else if (strcmp(opCode, "insert") == 0) {
		sprintf(sqlQuery, insert_sql, TABLENAME, uinfo.username, uinfo.password); 
		retCode = sqlite3_exec(db, sqlQuery, callBack, 0, &errorCode);
	} else if (strcmp(opCode, "update") == 0) {
		sprintf(where_clause, " user_name = \"%s\" and password = \"%s\" ", uinfo.username, uinfo.password);
		sprintf(sqlQuery, update_sql, TABLENAME, atoi(uinfo.total_scores), atoi(uinfo.best_score), atoi(uinfo.worst_score), atoi(uinfo.total_games), where_clause);
		retCode = sqlite3_exec(db, sqlQuery, callBack, 0, &errorCode);
	}
	
	fprintf(stdout, "The query is %s \n", sqlQuery);
	
	/* Execute Query on DB */
	if(retCode != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errorCode);
		sqlite3_free(errorCode);
		return 1;
	} else {
		fprintf(stdout, "Operation done successfully \n");
	}
	sqlite3_close(db);
	return 0;
}
