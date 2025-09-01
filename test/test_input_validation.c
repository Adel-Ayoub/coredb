#include "test_common.h"

// Test invalid inputs (negative IDs, zero IDs, etc.)
void test_invalid_inputs()
{
    Database db = setup_test_db("test.db");

    // Test 16: Insert negative ID
    int inserted = insert_row(&db, -1, "Invalid");
    struct Row rows[MAX_ROWS * MAX_PAGES];
    int count = select_rows(&db, rows, MAX_ROWS * MAX_PAGES);
    log_test(16, "Should not insert negative ID -1", inserted == 0 && count == 0);

    // Test 17: Insert zero ID
    inserted = insert_row(&db, 0, "Invalid");
    count = select_rows(&db, rows, MAX_ROWS * MAX_PAGES);
    log_test(17, "Should not insert zero ID", inserted == 0 && count == 0);

    // Test 18: Insert valid row, then duplicate
    inserted = insert_row(&db, 1, "Alice");
    count = select_rows(&db, rows, MAX_ROWS * MAX_PAGES);
    log_test(18, "Should insert first row with ID 1", inserted == 1 && count == 1 && rows[0].id == 1 && strcmp(rows[0].name, "Alice") == 0);

    inserted = insert_row(&db, 1, "Duplicate");
    count = select_rows(&db, rows, MAX_ROWS * MAX_PAGES);
    log_test(19, "Should not insert duplicate ID 1", inserted == 0 && count == 1 && rows[0].id == 1 && strcmp(rows[0].name, "Alice") == 0);

    // Test 20: Fill all pages to test max capacity (629 rows to reach 630 total)
    int successful_inserts = 0;
    for (unsigned long i = 2; i <= MAX_ROWS * MAX_PAGES; i++)
    { // 2 to 630 = 629 rows
        char name[60];
        snprintf(name, 60, "Name%lu", i);
        inserted = insert_row(&db, (int)i, name);
        if (inserted)
        {
            successful_inserts++;
        }
        else
        {
            log_test(20, "Should insert up to max rows", 0);
            // Don't return; continue to Test 21
        }
    }
    count = select_rows(&db, rows, MAX_ROWS * MAX_PAGES);
    log_test(20, "Should insert up to max rows", count == MAX_ROWS * MAX_PAGES && successful_inserts == MAX_ROWS * MAX_PAGES - 1);

    // Test 21: Insert after max rows reached
    inserted = insert_row(&db, MAX_ROWS * MAX_PAGES + 1, "TooMany");
    count = select_rows(&db, rows, MAX_ROWS * MAX_PAGES);
    log_test(21, "Should not insert after max rows reached", inserted == 0 && count == MAX_ROWS * MAX_PAGES);

    cleanup_test_db(&db, "test.db");
}
