#include "database.h"
#include <sqlite3.h>

int init_db()
{
    sqlite3 *db;
    char *err_msg = 0;
    int rc = sqlite3_open("credentials.db", &db);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    char *sql = "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT"
        "first_name TEXT NOT NULL"
        "last_name TEXT NOT NULL"
        "email TEXT UNIQUE NOT NULL"
        "password TEXT NOT NULL);"
        "todolist TEXT"; // the text will be json, so parse.
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Error when trying to execute (SQL): %s\n", err_msg);
        sqlite3_free(err_msg);
    } else {
        printf("Table created successfully.\n");
    }

    sqlite3_close(db);
    return 0;
}
