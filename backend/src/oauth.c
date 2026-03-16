#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>
#include "../include/oauth.h"

#define STR_SIZE 10000


CURL *curl; 

void rand_str(char *dest, size_t length) {
    char charset[] = "0123456789"
                     "abcdefghijklmnopqrstuvwxyz"
                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    while (length-- > 0) {
        size_t index = (double) rand() / RAND_MAX * (sizeof charset - 1);
        *dest++ = charset[index];
    }
    *dest = '\0';
}

char *build_github_auth_url(void) {
  char *url = "https://github.com/login/oauth/authorize?";
  const char *gh_client_id = getenv("GITHUB_CLIENT_ID");
  const char *redirect_uri = getenv("callback");

  char state[65];
  rand_str(state, sizeof state);

  snprintf(url, STR_SIZE,
      "client_id=%s&redirect_uri=%s&scope=read:user+user:email&state=%s",
      gh_client_id, redirect_uri, state);

  return url;
}

char *build_google_auth_url(void);
