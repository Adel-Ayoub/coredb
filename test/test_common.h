#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include "../include/coredb.h"

// Test logging with colors
#define GREEN "\033[32m"
#define RED "\033[31m"
#define PURPLE "\033[35m"
#define RESET "\033[0m"

// Test statistics
extern int total_tests;
extern int passed_tests;

// Test utility functions
void log_test(int test_num, const char *message, int passed);
void print_test_summary(void);

// Test database setup/teardown
Database setup_test_db(const char *filename);
void cleanup_test_db(Database *db, const char *filename);

// Test data helpers
void create_test_rows(Database *db, int start_id, int count);
void verify_row_exists(Database *db, int id, const char *expected_name);

#endif // TEST_COMMON_H
