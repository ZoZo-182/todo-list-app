#ifndef USER_DB_H
#define USER_DB_H

#include <sqlite3.h>
#include "request_handling.h"


int is_valid_password(char *password);
char* hash_password(char *password);

bool insert_user(sqlite3* db, ConnInfo *user_info);

char* check_user(sqlite3 *db, ConnInfo *user_info);

// add password + email constraints

#endif
