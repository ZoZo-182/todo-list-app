#include "../include/user_creds.h"
#include <microhttpd.h>
#include <sodium.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef TEST_BUILD
sqlite3 *db = NULL;
#else
extern sqlite3 *db;
#endif

char *hash_password(char *password) {
  char *hashed_password = malloc(crypto_pwhash_STRBYTES);
  if (!hashed_password) {
    return NULL;
  }

  if (crypto_pwhash_str(hashed_password, password, strlen(password),
                        crypto_pwhash_OPSLIMIT_INTERACTIVE,
                        crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
    fprintf(stderr, "error hashing password");
    free(hashed_password);
    return NULL;
  }

  return hashed_password; // remember to free this after function call
}

bool insert_user(sqlite3 *db, ConnInfo *user_info) {
//  char *err_msg = 0;
  sqlite3_stmt *statement;
  char *hashed_password = hash_password(user_info->password);

  // parameter binding instead of using sprintf (unsafe!)
  const char *sql = "INSERT INTO users (first_name, last_name, email,"
                    "password) VALUES (?, ?, ?, ?)";

  int rc = sqlite3_prepare_v2(db, sql, strlen(sql), &statement, NULL);
  if (rc == SQLITE_OK) {
    sqlite3_bind_text(statement, 1, user_info->first_name, -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, user_info->last_name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 3, user_info->email, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 4, hashed_password, -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(statement);
    if (rc != SQLITE_DONE) {
      fprintf(stderr, "error executing sql statement: %s (insert_user)\n",
              sqlite3_errmsg(db));
      sqlite3_finalize(statement);
      return false;
    }
    sqlite3_finalize(statement);
    free(hashed_password);
    return true;
  } else {
    fprintf(stderr, "Cannot add %s, %s, %s, and %s to db: %s\n",
            user_info->first_name, user_info->last_name, user_info->email,
            hashed_password, sqlite3_errmsg(db));
    printf("couldnt add user.");
    free(hashed_password);
    return false;
  }
}

char* check_user(sqlite3 *db, ConnInfo *user_info) {
  sqlite3_stmt *statement;

  // parameter binding instead of using sprintf (unsafe!)
  const char *sql = "SELECT email, password FROM users WHERE email= ?";

  int rc = sqlite3_prepare_v2(db, sql, strlen(sql), &statement, NULL);
  if (rc == SQLITE_OK) {
      // compare client user to user creds in database
    sqlite3_bind_text(statement, 1, user_info->email, -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(statement);
    if (rc != SQLITE_ROW) {
      fprintf(stderr, "error executing sql statement: %s (check_user)\n",
              sqlite3_errmsg(db));
      sqlite3_finalize(statement);
      return "incorrect email";
    }
    // check if no rows returned. 
   // if (!sqlite3_data_count(statement)) {
   //     sqlite3_finalize(statement);
   //     return "incorrect email";
   // }


    int correct_password = crypto_pwhash_str_verify(sqlite3_column_text(statement, 1), user_info->password,
            strlen(user_info->password));
    if (correct_password) {
        sqlite3_finalize(statement);
        return "incorrect password";
    }

    sqlite3_finalize(statement);
    return "successful login";
  } else {
    return "server error (sqlite3_prepare_v2)";
  }
}

static enum MHD_Result post_iterator(void *cls, enum MHD_ValueKind kind,
                                     const char *key, const char *filename,
                                     const char *content_type,
                                     const char *transfer_encoding,
                                     const char *data, uint64_t off,
                                     size_t size) {
  ConnInfo *user_info = cls;

  // put data received in user_info
  if (0 == strcmp("first_name", key)) {
    user_info->first_name = strndup(data, size);
    if (!user_info->first_name) {
      fprintf(stderr, "error allocating using strndup (post_iterator)");
      return MHD_NO;
    }
  }
  if (0 == strcmp("last_name", key)) {
    user_info->last_name = strndup(data, size);
    if (!user_info->last_name) {
      fprintf(stderr, "error allocating using strndup (post_iterator)");
      return MHD_NO;
    }
  }
  if (0 == strcmp("email", key)) {
    user_info->email = strndup(data, size);
    if (!user_info->email) {
      fprintf(stderr, "error allocating using strndup (post_iterator)");
      return MHD_NO;
    }
  }
  if (0 == strcmp("password", key)) {
    user_info->password = strndup(data, size);
    if (!user_info->password) {
      fprintf(stderr, "error allocating using strndup (post_iterator)");
      return MHD_NO;
    }
  }

  return MHD_YES;
}

static enum MHD_Result register_user(void *cls, struct MHD_Connection *connection,
                         const char *url, const char *method,
                         const char *version, const char *upload_data,
                         size_t *upload_data_size, void **con_cls) {
  struct MHD_Response *response;
  int ret;

  if (strcmp(method, "OPTIONS") == 0) {
      response = MHD_create_response_from_buffer(0, "", MHD_RESPMEM_PERSISTENT);

      MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
      MHD_add_response_header(response, "Access-Control-Allow-Methods", "POST, GET, OPTIONS");
      MHD_add_response_header(response, "Access-Control-Allow-Headers", "Content-Type");
      ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
      MHD_destroy_response(response);
      return ret;
  }

  ConnInfo *user_info;
  if(*con_cls == NULL) {
      user_info = malloc(sizeof(ConnInfo));
      if (!user_info) {
          return MHD_NO;
      }
      memset(user_info, 0, sizeof(ConnInfo));
      *con_cls = user_info;
  } else {
      user_info = *con_cls;
  }


  if (strcmp(url, "/register") == 0 && strcmp(method, "POST") == 0) {
    if (user_info->pp == NULL) {
      user_info->pp = MHD_create_post_processor(connection, 1024,
                                                &post_iterator, user_info);
      return MHD_YES;
    }
    if (*upload_data_size) {
      MHD_post_process(user_info->pp, upload_data, *upload_data_size);
      *upload_data_size = 0;
      return MHD_YES;
    } else {
      if (!user_info->first_name || !user_info->last_name ||
          !user_info->email || !user_info->password) {
        // send response
        const char *msg = "one or more of the user_info fields are empty";
        response = MHD_create_response_from_buffer(strlen(msg), (void *) msg, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
        MHD_add_response_header(response, "Access-Control-Allow-Methods", "POST, GET, OPTIONS");
        MHD_add_response_header(response, "Access-Control-Allow-Headers", "Content-Type");
        ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);

        MHD_destroy_response(response);
        MHD_destroy_post_processor(user_info->pp);
        free(user_info->first_name);
        free(user_info->last_name);
        free(user_info->email);
        free(user_info->password);
        free(user_info);

        return ret;
      }

     // int rc = sqlite3_open("credentials.db", &db);
     // if (rc != SQLITE_OK) {
     //   fprintf(stderr, "error opening database: %s (register_user)\n",
     //           sqlite3_errmsg(db));
     //   const char *msg = "error opening database.";
     //   response = MHD_create_response_from_buffer(strlen(msg), (void *) msg, MHD_RESPMEM_PERSISTENT);
     //   MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
     //   ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);

     //   MHD_destroy_response(response);
     //   MHD_destroy_post_processor(user_info->pp);
     //   free(user_info->first_name);
     //   free(user_info->last_name);
     //   free(user_info->email);
     //   free(user_info->password);
     //   free(user_info);
     //   return ret;
     // }
      bool inserted_user = insert_user(db, user_info);
      if (!inserted_user) {
          const char *msg = "Failed to register user.";
          response = MHD_create_response_from_buffer(strlen(msg), (void *)msg, MHD_RESPMEM_PERSISTENT);
          MHD_add_response_header(response, "Access-Control-Allow-Origin","*");
          MHD_add_response_header(response, "Access-Control-Allow-Methods", "POST, GET, OPTIONS");
          MHD_add_response_header(response, "Access-Control-Allow-Headers", "Content-Type");
          ret = MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
          MHD_destroy_response(response);

     //     sqlite3_close(db);

          MHD_destroy_post_processor(user_info->pp);
          free(user_info->first_name);
          free(user_info->last_name);
          free(user_info->email);
          free(user_info->password);
          free(user_info);
          return ret;
      }
      // sqlite3_close(db);

      const char *msg = "User Registered.";
      response = MHD_create_response_from_buffer(strlen(msg), (void *) msg, MHD_RESPMEM_PERSISTENT);
      
      MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
      MHD_add_response_header(response, "Access-Control-Allow-Methods", "POST, GET, OPTIONS");
      MHD_add_response_header(response, "Access-Control-Allow-Headers", "Content-Type");

      ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
      MHD_destroy_response(response);

      MHD_destroy_post_processor(user_info->pp);
      free(user_info->first_name);
      free(user_info->last_name);
      free(user_info->email);
      free(user_info->password);
      free(user_info);

      // return MHD_queue_response(connection, code, response);
      return ret;
    }
  } else if (strcmp(method, "POST") == 0 && strcmp(url, "/login")) {
      char *checked_user = check_user(db, user_info);
      if (!checked_user) {
          const char *msg = "login failed.";
          response = MHD_create_response_from_buffer(strlen(msg), (void *)msg, MHD_RESPMEM_PERSISTENT);
          MHD_add_response_header(response, "Access-Control-Allow-Origin","*");
          MHD_add_response_header(response, "Access-Control-Allow-Methods", "POST, GET, OPTIONS");
          MHD_add_response_header(response, "Access-Control-Allow-Headers", "Content-Type");
          ret = MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
          MHD_destroy_response(response);

     //     sqlite3_close(db);

          MHD_destroy_post_processor(user_info->pp);
          free(user_info->first_name);
          free(user_info->last_name);
          free(user_info->email);
          free(user_info->password);
          free(user_info);
          return ret;
      }
      // sqlite3_close(db);

      const char *msg = "User Registered.";
      response = MHD_create_response_from_buffer(strlen(msg), (void *) msg, MHD_RESPMEM_PERSISTENT);
      
      MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
      MHD_add_response_header(response, "Access-Control-Allow-Methods", "POST, GET, OPTIONS");
      MHD_add_response_header(response, "Access-Control-Allow-Headers", "Content-Type");

      ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
      MHD_destroy_response(response);

      MHD_destroy_post_processor(user_info->pp);
      free(user_info->first_name);
      free(user_info->last_name);
      free(user_info->email);
      free(user_info->password);
      free(user_info);
          // do stuff
  } else {
      const char *not_found = "Not Found";
      response = MHD_create_response_from_buffer(strlen(not_found), (void*) not_found, MHD_RESPMEM_PERSISTENT);
      ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
      MHD_destroy_response(response);

      free(user_info->first_name);
      free(user_info->last_name);
      free(user_info->email);
      free(user_info->password);
      free(user_info);
      return ret;
  }
  return MHD_NO;
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

  if (argc != 2) {
    printf("%s PORT\n", argv[0]);
    return 1;
  }
  /* initialize PRNG */
  srandom((unsigned int)time(NULL));
  d = MHD_start_daemon(MHD_USE_DEBUG, atoi(argv[1]), NULL, NULL, &register_user, NULL,
                       MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int)15,
                       MHD_OPTION_NOTIFY_COMPLETED, NULL, NULL, MHD_OPTION_END);
  if (NULL == d)
    return 1;
  while (1) {
    max = 0;
    FD_ZERO(&rs);
    FD_ZERO(&ws);
    FD_ZERO(&es);
    if (MHD_YES != MHD_get_fdset(d, &rs, &ws, &es, &max))
      break; /* fatal internal error */
    if (MHD_get_timeout(d, &mhd_timeout) == MHD_YES) {
      tv.tv_sec = mhd_timeout / 1000;
      tv.tv_usec = (mhd_timeout - (tv.tv_sec * 1000)) * 1000;
      tvp = &tv;
    } else
      tvp = NULL;
    select(max + 1, &rs, &ws, &es, tvp);
    MHD_run(d);
  }
  MHD_stop_daemon(d);
  return 0;
}

// add password + email constraints
