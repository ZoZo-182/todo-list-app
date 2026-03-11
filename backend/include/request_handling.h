#ifndef REQUEST_HANDLING_H
#define REQUEST_HANDLING_H

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

int MHD_background(int argc, char *const *argv); 

#endif
