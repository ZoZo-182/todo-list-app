#include "user_creds.h"
#include <microhttpd.h>
#include <string.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <sodium.h>

char* hash_password(char *password) {
  char *hashed_password = malloc(crypto_pwhash_STRBYTES);
  if (!hashed_password) {
      return NULL;
  }

  if (crypto_pwhash_str(
              hashed_password,
              password,
              strlen(password),
              crypto_pwhash_OPSLIMIT_SENSITIVE,
              crypto_pwhash_MEMLIMIT_SENSITIVE) != 0) {
      fprintf(stderr,"error hashing password");
      free(hashed_password);
      return NULL;
  }

  return hashed_password; // remember to free this after function call
}

bool insert_user(sqlite3 *db, ConnInfo *user_info) {
  char *err_msg = 0;
  sqlite3_stmt *statement;

  // parameter binding instead of using sprintf (unsafe!)
  const char *sql = "INSERT INTO users (first_name, last_name, email,"
        "password) VALUES (?, ?, ?, ?)";

  int rc = sqlite3_prepare_v2(db, sql, strlen(sql),
                               &statement, NULL);
  if (rc == SQLITE_OK) {
    sqlite3_bind_text(statement, 1, user_info->first_name, -1,
            SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, user_info->last_name, -1,
            SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 3, user_info->first_name, -1,
            SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 4, user_info->first_name, -1,
            SQLITE_TRANSIENT);
    rc = sqlite3_step(statement);
    return true;
  } else {
    fprintf(stderr, "Cannot add %s, %s, %s, and %s to db: %s\n",
            user_info->first_name, user_info->last_name, user_info->email,
            hash_password(user_info->password), sqlite3_errmsg(db));
    printf("couldnt add user.");
    return false;
  }
}

static enum MHD_Result post_iterator(void *cls, enum MHD_ValueKind kind, const char *key,
        const char *filename, const char *content_type,
        const char *transfer_encoding, const char *data, uint64_t off,
        size_t size)
{
    ConnInfo *user_info = cls;
    bool success;
    // put data received in user_info
    if (0 == strcmp("first_name", key))
    {
        user_info->first_name = data;
    }
    if (0 == strcmp("last_name", key))
    {
        user_info->last_name = data;
    }
    if (0 == strcmp("email", key))
    {
        user_info->email = data;
    }
    if (0 == strcmp("password", key))
    {
        user_info->password = data;
    }

    sqlite3 *db;
    // db connection 
    success = insert_user(db, user_info);
    if (success != true) {
        printf("error inserting user.");
        return MHD_NO;
    }
    
    return MHD_YES;
}

static int register_user(void *cls, struct MHD_Connection *connection,
        const char *url, const char *method, const char *version,
        const char *upload_data, size_t *upload_data_size,
        void **con_cls)
{
    ConnInfo *user_info = *con_cls;

    if (user_info->pp == NULL)
    {
        user_info->pp = MHD_create_post_processor(connection, *upload_data_size, 
                &post_iterator, user_info);
        *con_cls = user_info->pp;
        return MHD_YES;
    }
    if (*upload_data_size)
    {
        MHD_post_process(user_info->pp, upload_data, *upload_data_size);
        *upload_data_size = 0;
        return MHD_YES;
    }
    else 
    {
        MHD_destroy_post_processor(user_info->pp);
        //return MHD_queue_response(connection, code, response);
        return MHD_NO;
    }
}

int MHD_background(int argc, char *const *argv) {
  struct MHD_Daemon *d;
  struct timeval tv;
  struct timeval *tvp;
  fd_set rs;
  fd_set ws;
  fd_set es;
  MHD_socket max;
  MHD_UNSIGNED_LONG_LONG mhd_timeout;

  if (argc != 2)
    {
      printf ("%s PORT\n", argv[0]);
      return 1;
    }
  /* initialize PRNG */
  srandom((unsigned int) time (NULL));
  d = MHD_start_daemon (MHD_USE_DEBUG,
                        atoi (argv[1]),
                        NULL, NULL,
			NULL, NULL,
			MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) 15,
			MHD_OPTION_NOTIFY_COMPLETED, NULL, NULL,
			MHD_OPTION_END);
  if (NULL == d)
    return 1;
  while (1)
    {
      max = 0;
      FD_ZERO (&rs);
      FD_ZERO (&ws);
      FD_ZERO (&es);
      if (MHD_YES != MHD_get_fdset (d, &rs, &ws, &es, &max))
	break; /* fatal internal error */
      if (MHD_get_timeout (d, &mhd_timeout) == MHD_YES)
	{
	  tv.tv_sec = mhd_timeout / 1000;
	  tv.tv_usec = (mhd_timeout - (tv.tv_sec * 1000)) * 1000;
	  tvp = &tv;
	}
      else
	tvp = NULL;
      select (max + 1, &rs, &ws, &es, tvp);
      MHD_run (d);
    }
  MHD_stop_daemon (d);
}
// login_user function
// parse the data into the struct user_info
// open db connection
// do checks like if email exists in db
// if yes then check if password is correct
// send back responses accordingly
