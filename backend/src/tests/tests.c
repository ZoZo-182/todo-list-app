#include "../../munit/munit.h"
#include "../../user_creds.h"
#include <sodium.h>

// create db
sqlite3 *db;
ConnInfo user = {NULL, "Bob", "Smith", "chickenbirriataco@gmail.com", "yummy"};
// create a fake user for db
// test if insert is true 

MunitResult test_pwhash(const MunitParameter params[], void* user_data_or_fixture) {
    char *result = hash_password("yummy");
    printf("hash_password: %s\n", result);
    munit_assert_string_not_equal(result, "");
    
    free(result);
    return MUNIT_OK;
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
