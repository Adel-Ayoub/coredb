#include "test_common.h"

// Test function declarations
void test_insert_select_delete(void);
void test_select_by_id(void);
void test_unique_id_enforcement(void);
void test_invalid_inputs(void);
void test_update(void);
void test_compaction(void);

int main()
{
    total_tests = 0;
    passed_tests = 0;
    
    printf("Running coredb test suite...\n");
    printf("================================\n");
    
    // Run all test suites
    test_insert_select_delete();
    test_select_by_id();
    test_unique_id_enforcement();
    test_invalid_inputs();
    test_update();
    test_compaction();
    
    printf("================================\n");
    print_test_summary();
    
    return 0;
}
