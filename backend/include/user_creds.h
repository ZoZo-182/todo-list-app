#ifndef USER_CREDS_H
#define USER_CREDS_H


#include <stdio.h>
#include <sqlite3.h>
#include <stdbool.h>

// struct for user register data
typedef struct 
{
    struct MHD_PostProcessor *pp;
    char *first_name;
    char *last_name;
    char *email;
    char *password;
} ConnInfo;

int is_valid_password(char *password);
char* hash_password(char *password);

bool insert_user(sqlite3* db, ConnInfo *user_info);

int MHD_background(int argc, char *const *argv); 
    

// login function 
int login();

#endif
