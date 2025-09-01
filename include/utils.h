#ifndef UTILS_H
#define UTILS_H

#include "coredb.h"

// Utility functions for data validation and formatting
int is_valid_id(int id);
void format_row_name(char *dest, const char *src, size_t max_len);

#endif // UTILS_H
