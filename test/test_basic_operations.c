#include "test_common.h"

// Test insert, select, delete
void test_insert_select_delete()
{
    Database db = setup_test_db("test.db");

    // Test 1: Empty database
    struct Row rows[MAX_ROWS * MAX_PAGES];
    int count = select_rows(&db, rows, MAX_ROWS * MAX_PAGES);
    log_test(1, "Empty database should have 0 rows", count == 0);

    // Test 2: Insert one row and select
    int inserted = insert_row(&db, 1, "Alice");
    count = select_rows(&db, rows, MAX_ROWS * MAX_PAGES);
    log_test(2, "Should have 1 row after insert", inserted == 1 && count == 1 && rows[0].id == 1 && strcmp(rows[0].name, "Alice") == 0);

    // Test 3: Insert rows to fill first page (63 rows)
    for (unsigned long i = 2; i <= MAX_ROWS; i++)
    {
        char name[60];
        snprintf(name, 60, "Name%lu", i);
        inserted = insert_row(&db, (int)i, name);
        if (!inserted)
        {
            log_test(3, "Should insert up to 63 rows", 0);
            cleanup_test_db(&db, "test.db");
            return;
        }
    }
    count = select_rows(&db, rows, MAX_ROWS * MAX_PAGES);
    log_test(3, "Should have 63 rows after filling first page", count == MAX_ROWS);

    // Test 4: Insert row to trigger new page
    inserted = insert_row(&db, 64, "NewPage");
    count = select_rows(&db, rows, MAX_ROWS * MAX_PAGES);
    log_test(4, "Should have 64 rows after new page", inserted == 1 && count == MAX_ROWS + 1 && rows[MAX_ROWS].id == 64 && strcmp(rows[MAX_ROWS].name, "NewPage") == 0);

    // Test 5: Delete a row and select
    int deleted = delete_row(&db, 1);
    count = select_rows(&db, rows, MAX_ROWS * MAX_PAGES);
    log_test(5, "Should have 63 rows after delete", deleted == 1 && count == MAX_ROWS);

    // Test 6: Persistence after restart
    close_db(&db);
    db = init_db("test.db");
    count = select_rows(&db, rows, MAX_ROWS * MAX_PAGES);
    log_test(6, "Should have 63 rows after restart", count == MAX_ROWS);

    cleanup_test_db(&db, "test.db");
}
