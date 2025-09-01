#ifndef BTREE_H
#define BTREE_H

#include "coredb.h"

// B-Tree node operations
void read_node(Database *db, off_t offset, BTreeNode *node);
void write_node(Database *db, off_t offset, BTreeNode *node);
off_t allocate_node(Database *db);

// B-Tree operations
void btree_search(Database *db, int id, off_t *address);
void btree_insert(Database *db, int id, off_t address);
void btree_delete(Database *db, int id);

#endif // BTREE_H
