#include "test_common.h"

// Test unique ID enforcement
void test_unique_id_enforcement()
{
    Database db = setup_test_db("test.db");

    // Test 13: Insert first row
    int inserted = insert_row(&db, 1, "Alice");
    struct Row rows[MAX_ROWS * MAX_PAGES];
    int count = select_rows(&db, rows, MAX_ROWS * MAX_PAGES);
    log_test(13, "Should insert first row with ID 1", inserted == 1 && count == 1 && rows[0].id == 1 && strcmp(rows[0].name, "Alice") == 0);

    // Test 14: Insert duplicate ID
    inserted = insert_row(&db, 1, "Bob");
    count = select_rows(&db, rows, MAX_ROWS * MAX_PAGES);
    log_test(14, "Should not insert duplicate ID 1", inserted == 0 && count == 1 && rows[0].id == 1 && strcmp(rows[0].name, "Alice") == 0);

    // Test 15: Insert new ID after duplicate attempt
    inserted = insert_row(&db, 2, "Bob");
    count = select_rows(&db, rows, MAX_ROWS * MAX_PAGES);
    log_test(15, "Should insert new row with ID 2", inserted == 1 && count == 2 && rows[0].id == 1 && rows[1].id == 2 && strcmp(rows[1].name, "Bob") == 0);

    cleanup_test_db(&db, "test.db");
}
