#ifndef COREDB_H
#define COREDB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Constants
#define PAGE_SIZE 4096
#define MAX_ROWS ((PAGE_SIZE - sizeof(int)) / sizeof(struct Row))
#define MAX_PAGES 10
#define INDEX_PAGES 10
#define HEADER_PAGES 1
#define DATA_START_OFFSET ((HEADER_PAGES + INDEX_PAGES) * PAGE_SIZE)
#define MAX_KEYS 255
#define MAX_CHILDREN 256

// Core data structures
struct Row {
    int id;
    char name[60];
};

typedef struct {
    int id;
    off_t address;
} IndexEntry;

typedef struct {
    int num_keys;
    int is_leaf;
    union {
        struct {
            IndexEntry entries[MAX_KEYS];
        } leaf;
        struct {
            int keys[MAX_KEYS];
            off_t children[MAX_CHILDREN];
        } internal;
    } data;
} BTreeNode;

typedef struct {
    FILE *file;
    void **pages;
    int num_pages;
    int max_pages;
    off_t root_offset;
    int page_dirty[MAX_PAGES];
} Database;

// Function declarations will be included from other headers
#include "database.h"
#include "btree.h"
#include "crud.h"
#include "storage.h"
#include "utils.h"
#include "repl.h"

#endif // COREDB_H
