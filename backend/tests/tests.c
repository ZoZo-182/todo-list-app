#include "../lib/munit/munit.h"
#include "../include/user_creds.h"
#include <sodium.h>

ConnInfo user = {NULL, "Bob", "Smith", "chickenbirriataco@gmail.com", "yummy"};

MunitResult test_pwhash(const MunitParameter params[], void* user_data_or_fixture) {
    char *result = hash_password(user.password);
    printf("hash_password: %s\n", result);
    munit_assert_string_not_equal(result, "");
    
    free(result);
    return MUNIT_OK;
}


MunitResult test_insert_user(const MunitParameter params[], void* user_data_or_fixture) {
    sqlite3 *db = (sqlite3 *)user_data_or_fixture;
    munit_assert_not_null(db);

    bool result = insert_user(db, &user);
    printf("user inserted: %b\n", result);
    munit_assert_true(result);
    return MUNIT_OK;
}

static void* test_setup(const MunitParameter params[], void* user_dat) {
    char *err_msg = 0;

    sqlite3 *test_db;
    int rc = sqlite3_open(":memory:", &test_db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "error opening db: %s\n", sqlite3_errmsg(test_db));
        return NULL;
    }

  char *sql = "CREATE TABLE IF NOT EXISTS users ("
              "id INTEGER PRIMARY KEY AUTOINCREMENT,"
              "first_name TEXT NOT NULL,"
              "last_name TEXT NOT NULL,"
              "email TEXT UNIQUE NOT NULL,"
              "password TEXT NOT NULL,"
              "todolist TEXT)"; // the text will be json, so parse.
  rc = sqlite3_exec(test_db, sql, 0, 0, &err_msg);

  if (rc != SQLITE_OK) {
    fprintf(stderr, "Error when trying to execute (SQL): %s\n", err_msg);
    sqlite3_free(err_msg);
  } else {
    printf("Table created successfully.\n");
  }
    
    return test_db;
}

static void test_tear_down(void *fixture) {
    sqlite3 *test_db = (sqlite3*)fixture;
    if (test_db) {
        sqlite3_close(test_db);
    }
}

static MunitTest tests[] = {
    {
    "/hash-pw-test",
    test_pwhash,
    NULL,
    NULL,
    MUNIT_TEST_OPTION_NONE,
    NULL
    },
    {
    "/insert-user-test",
    test_insert_user,
    test_setup,
    test_tear_down,
    MUNIT_TEST_OPTION_NONE,
    NULL
    },
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite suite = {
    "/my-tests",
    tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

int main(int argc, const char* argv[]) {
    if (sodium_init() < 0) {
        printf("sodium lib not initalized");
    }
    return munit_suite_main(&suite, NULL, argc, argv);
}
