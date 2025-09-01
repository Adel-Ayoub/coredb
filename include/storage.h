#ifndef STORAGE_H
#define STORAGE_H

#include "coredb.h"

// File I/O operations
int read_page_from_file(Database *db, int page_num);
int write_page_to_file(Database *db, int page_num);
void flush_all_pages(Database *db);

#endif // STORAGE_H
