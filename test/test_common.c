#include "test_common.h"
#include <stdio.h>

// Global test statistics
int total_tests = 0;
int passed_tests = 0;

// Test logging with colors
void log_test(int test_num, const char *message, int passed)
{
    total_tests++;
    if (passed)
    {
        passed_tests++;
        printf("Test %d %s[PASSED] %s%s\n", test_num, GREEN, message, RESET);
    }
    else
    {
        printf("Test %d %s[FAILED] %s%s\n", test_num, RED, message, RESET);
    }
}

// Print test summary
void print_test_summary(void)
{
    printf("%s%d/%d tests passed!%s\n", PURPLE, passed_tests, total_tests, RESET);
}

// Setup test database
Database setup_test_db(const char *filename)
{
    remove(filename); // Ensure clean state
    return init_db(filename);
}

// Cleanup test database
void cleanup_test_db(Database *db, const char *filename)
{
    close_db(db);
    remove(filename); // Clean up test file
}

// Create test rows for testing
void create_test_rows(Database *db, int start_id, int count)
{
    for (int i = 0; i < count; i++)
    {
        char name[60];
        snprintf(name, 60, "Name%d", start_id + i);
        insert_row(db, start_id + i, name);
    }
}

// Verify a row exists with expected values
void verify_row_exists(Database *db, int id, const char *expected_name)
{
    struct Row row;
    int found = select_by_id(db, id, &row);
    if (found && strcmp(row.name, expected_name) == 0)
    {
        printf("Verified row %d: %s\n", id, expected_name);
    }
    else
    {
        printf("Failed to verify row %d\n", id);
    }
}
