#ifndef CRUD_H
#define CRUD_H

#include "coredb.h"

// Create, Read, Update, Delete operations
int insert_row(Database *db, int id, const char *name);
int select_rows(Database *db, struct Row *rows, int max_rows);
int select_by_id(Database *db, int id, struct Row *row);
int update_row(Database *db, int id, const char *name);
int delete_row(Database *db, int id);
void compact_pages(Database *db);

#endif // CRUD_H
