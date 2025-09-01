#ifndef DATABASE_H
#define DATABASE_H

#include "coredb.h"

// Database lifecycle functions
Database init_db(const char *filename);
void close_db(Database *db);

// Database state management
void write_buffer(Database *db);

#endif // DATABASE_H
