#include "test_common.h"

// Test update functionality
void test_update()
{
    Database db = setup_test_db("test.db");

    // Test 22: Insert a row to update
    int inserted = insert_row(&db, 1, "Alice");
    struct Row rows[MAX_ROWS * MAX_PAGES];
    int count = select_rows(&db, rows, MAX_ROWS * MAX_PAGES);
    log_test(22, "Should insert row with ID 1", inserted == 1 && count == 1 && rows[0].id == 1 && strcmp(rows[0].name, "Alice") == 0);

    // Test 23: Update existing row
    int updated = update_row(&db, 1, "Bob");
    count = select_rows(&db, rows, MAX_ROWS * MAX_PAGES);
    log_test(23, "Should update row with ID 1 to Bob", updated == 1 && count == 1 && rows[0].id == 1 && strcmp(rows[0].name, "Bob") == 0);

    // Test 24: Update non-existent row
    updated = update_row(&db, 2, "Charlie");
    count = select_rows(&db, rows, MAX_ROWS * MAX_PAGES);
    log_test(24, "Should not update non-existent row with ID 2", updated == 0 && count == 1 && rows[0].id == 1 && strcmp(rows[0].name, "Bob") == 0);

    // Test 25: Update with invalid ID
    updated = update_row(&db, -1, "Invalid");
    count = select_rows(&db, rows, MAX_ROWS * MAX_PAGES);
    log_test(25, "Should not update with invalid ID -1", updated == 0 && count == 1 && rows[0].id == 1 && strcmp(rows[0].name, "Bob") == 0);

    // Test 26: Persistence after restart
    close_db(&db);
    db = init_db("test.db");
    count = select_rows(&db, rows, MAX_ROWS * MAX_PAGES);
    log_test(26, "Should retain updated row after restart", count == 1 && rows[0].id == 1 && strcmp(rows[0].name, "Bob") == 0);

    cleanup_test_db(&db, "test.db");
}
