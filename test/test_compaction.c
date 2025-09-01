#include "test_common.h"

// Test compaction functionality
void test_compaction()
{
    Database db = setup_test_db("test.db");

    // Test 27: Insert multiple rows and delete middle one
    int inserted = insert_row(&db, 1, "Alice");
    inserted &= insert_row(&db, 2, "Bob");
    inserted &= insert_row(&db, 3, "Charlie");
    struct Row rows[MAX_ROWS * MAX_PAGES];
    int count = select_rows(&db, rows, MAX_ROWS * MAX_PAGES);
    log_test(27, "Should insert 3 rows", inserted == 1 && count == 3 && rows[0].id == 1 && rows[1].id == 2 && rows[2].id == 3);

    int deleted = delete_row(&db, 2);
    count = select_rows(&db, rows, MAX_ROWS * MAX_PAGES);
    log_test(28, "Should compact rows after deleting ID 2", deleted == 1 && count == 2 && rows[0].id == 1 && rows[1].id == 3 && strcmp(rows[0].name, "Alice") == 0 && strcmp(rows[1].name, "Charlie") == 0);

    // Test 29: Fill a page, delete all, verify page removal
    // Insert rows
    for (unsigned long i = 4; i <= MAX_ROWS + 3; i++)
    {
        char name[60];
        snprintf(name, 60, "Name%lu", i);
        insert_row(&db, (int)i, name);
    }

    // Delete every other row (even IDs)
    for (unsigned long i = 4; i <= MAX_ROWS + 3; i++)
    {
        if (i % 2 == 0) {
            delete_row(&db, (int)i);
        }
    }
    count = select_rows(&db, rows, MAX_ROWS * MAX_PAGES);
    // Expect roughly half the rows to remain (odd IDs)
    log_test(29, "Should remove empty page and retain roughly half rows", count > 30 && count < 50 && db.num_pages == 1);

    // Test 30: Insert after compaction
    inserted = insert_row(&db, 4, "David");
    int new_count = select_rows(&db, rows, MAX_ROWS * MAX_PAGES);
    log_test(30, "Should insert new row after compaction", inserted == 1 && new_count == count + 1);

    cleanup_test_db(&db, "test.db");
}
