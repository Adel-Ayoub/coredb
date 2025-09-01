#include "test_common.h"

// Test select by ID
void test_select_by_id()
{
    Database db = setup_test_db("test.db");

    // Test 7: Empty database
    struct Row row;
    int found = select_by_id(&db, 1, &row);
    log_test(7, "Should not find any row in empty database", found == 0);

    // Test 8: Insert rows and select by ID
    int inserted = insert_row(&db, 1, "Alice");
    inserted &= insert_row(&db, 2, "Bob");
    inserted &= insert_row(&db, 100, "Charlie");
    if (!inserted)
    {
        log_test(8, "Failed to insert rows for select by ID test", 0);
        cleanup_test_db(&db, "test.db");
        return;
    }

    found = select_by_id(&db, 1, &row);
    log_test(8, "Should find row with ID 1", found == 1 && row.id == 1 && strcmp(row.name, "Alice") == 0);

    found = select_by_id(&db, 100, &row);
    log_test(9, "Should find row with ID 100", found == 1 && row.id == 100 && strcmp(row.name, "Charlie") == 0);

    // Test 10: Select non-existent ID
    found = select_by_id(&db, 999, &row);
    log_test(10, "Should not find row with ID 999", found == 0);

    // Test 11: Select deleted row
    int deleted = delete_row(&db, 2);
    found = select_by_id(&db, 2, &row);
    log_test(11, "Should not find deleted row with ID 2", deleted == 1 && found == 0);

    // Test 12: Persistence after restart
    close_db(&db);
    db = init_db("test.db");
    found = select_by_id(&db, 1, &row);
    log_test(12, "Should find row with ID 1 after restart", found == 1 && row.id == 1);

    cleanup_test_db(&db, "test.db");
}
