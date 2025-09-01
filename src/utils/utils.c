#include "../../include/coredb.h"

// Check if an ID is valid (positive integer)
int is_valid_id(int id)
{
    return id > 0;
}

// Format row name with proper length checking
void format_row_name(char *dest, const char *src, size_t max_len)
{
    strncpy(dest, src, max_len - 1);
    dest[max_len - 1] = '\0';
}
