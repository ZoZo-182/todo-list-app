#ifndef USER_CREDS_H
#define USER_CREDS_H


#include <stdio.h>
#define DATABASE "credentials.db"

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

void db_connection(char *, ...);


    

// login function 
int login();

#endif
