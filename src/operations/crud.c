#include "../../include/coredb.h"

// Insert a row (returns 1 if inserted, 0 if failed due to duplicate ID)
int insert_row(Database *db, int id, const char *name)
{
    if (id <= 0)
    {
        printf("Error: ID must be a positive integer (got %d)\n", id);
        return 0;
    }

    // Check for duplicate ID
    off_t address;
    btree_search(db, id, &address);
    if (address != -1)
    {
        printf("Error: Row with id=%d already exists\n", id);
        return 0;
    }

    int current_page = db->num_pages - 1;
    int *page_num_rows = (int *)db->pages[current_page]; // Pointer to the number of rows in the page
    if ((unsigned long)*page_num_rows >= MAX_ROWS)
    {
        if (db->num_pages >= db->max_pages)
        {
            printf("Error: Maximum pages reached, cannot insert more rows\n");
            return 0;
        }
        void *new_page = malloc(PAGE_SIZE);
        if (new_page == NULL)
        {
            printf("Error: Could not allocate new page\n");
            return 0;
        }
        memset(new_page, 0, PAGE_SIZE); // Initialize the new page to zero
        db->pages[db->num_pages] = new_page;
        db->num_pages++;
        current_page = db->num_pages - 1;
        page_num_rows = (int *)db->pages[current_page]; // Update pointer to the new page
    }

    struct Row new_row;
    new_row.id = id;
    strncpy(new_row.name, name, 59);
    new_row.name[59] = '\0';

    size_t offset = sizeof(int) + (*page_num_rows * sizeof(struct Row));
    memcpy((char *)db->pages[current_page] + offset, &new_row, sizeof(struct Row));

    (*page_num_rows)++;
    int updated_num_rows = *page_num_rows;
    memcpy(db->pages[current_page], &updated_num_rows, sizeof(int));

    // Compute the row's address in the file
    off_t row_address = DATA_START_OFFSET + (off_t)current_page * PAGE_SIZE + offset;

    // Insert into B-Tree
    btree_insert(db, id, row_address);

    db->page_dirty[current_page] = 1;
    write_buffer(db);
    return 1;
}

// select all rows, returns count of non-deleted rows
int select_rows(Database *db, struct Row *rows, int max_rows)
{
    int count = 0;
    for (int page = 0; page < db->num_pages && count < max_rows; page++)
    {
        int *page_num_rows = (int *)db->pages[page]; // Pointer to the number of rows in the page
        for (int i = 0; i < *page_num_rows && count < max_rows; i++)
        {
            size_t offset = sizeof(int) + (i * sizeof(struct Row));
            struct Row temp_row;
            memcpy(&temp_row, (char *)db->pages[page] + offset, sizeof(struct Row));
            if (temp_row.id != 0) // Check if row is not deleted
            {
                rows[count++] = temp_row;
            }
        }
    }
    return count;
}

// Select a row by ID (returns 1 if found, 0 if not)
int select_by_id(Database *db, int id, struct Row *row)
{
    if (id <= 0)
    {
        printf("Error: ID must be a positive integer (got %d)\n", id);
        return 0;
    }

    off_t address;
    btree_search(db, id, &address);
    if (address == -1)
    {
        printf("Error: Row with id=%d not found\n", id);
        return 0;
    }

    fseek(db->file, address, SEEK_SET);
    size_t bytes_read = fread(row, sizeof(struct Row), 1, db->file);
    if (bytes_read != 1)
    {
        printf("Error: Failed to read row at address %lld\n", (long long)address);
        return 0;
    }
    return 1;
}

// update a row
int update_row(Database *db, int id, const char *name)
{
    if (id <= 0)
    {
        printf("Error: ID must be a positive integer (got %d)\n", id);
        return 0;
    }

    off_t address;
    btree_search(db, id, &address);
    if (address == -1)
    {
        printf("Error: Row with id=%d not found\n", id);
        return 0;
    }

    struct Row row;
    fseek(db->file, address, SEEK_SET);
    size_t bytes_read = fread(&row, sizeof(struct Row), 1, db->file);
    if (bytes_read != 1)
    {
        printf("Error: Failed to read row at address %lld\n", (long long)address);
        return 0;
    }
    strncpy(row.name, name, 59);
    row.name[59] = '\0';
    fseek(db->file, address, SEEK_SET);
    fwrite(&row, sizeof(struct Row), 1, db->file);

    // update in memory pages
    for (int page = 0; page < db->num_pages; page++)
    {
        int *page_num_rows = (int *)db->pages[page];
        for (int i = 0; i < *page_num_rows; i++)
        {
            size_t offset = sizeof(int) + (i * sizeof(struct Row));
            off_t computed_address = DATA_START_OFFSET + (off_t)page * PAGE_SIZE + offset;

            if (computed_address == address)
            {
                memcpy((char *)db->pages[page] + offset, &row, sizeof(struct Row));
                db->page_dirty[page] = 1;
                break;
            }
        }
    }
    write_buffer(db);
    return 1;
}

// Compact pages by consolidating rows and removing empty pages
void compact_pages(Database *db)
{
    // Count total remaining rows
    int total_rows = 0;
    for (int p = 0; p < db->num_pages; p++)
    {
        int *page_num_rows = (int *)db->pages[p];
        total_rows += *page_num_rows;
    }

    if (total_rows == 0)
    {
        // All rows deleted, keep one empty page
        if (db->num_pages > 1)
        {
            for (int p = 1; p < db->num_pages; p++)
            {
                free(db->pages[p]);
                db->pages[p] = NULL;
            }
            db->num_pages = 1;
            memset(db->pages[0], 0, PAGE_SIZE);
            int zero = 0;
            memcpy(db->pages[0], &zero, sizeof(int));
            db->page_dirty[0] = 1;
        }
        return;
    }

    // Calculate how many pages we actually need
    int rows_per_page = (PAGE_SIZE - sizeof(int)) / sizeof(struct Row);
    int pages_needed = (total_rows + rows_per_page - 1) / rows_per_page; // Ceiling division

    // Collect all rows into a temporary array
    struct Row *all_rows = malloc(total_rows * sizeof(struct Row));
    if (all_rows == NULL)
    {
        printf("Error: Could not allocate memory for compaction\n");
        return;
    }

    int row_index = 0;
    for (int p = 0; p < db->num_pages; p++)
    {
        int *page_num_rows = (int *)db->pages[p];
        for (int r = 0; r < *page_num_rows; r++)
        {
            size_t offset = sizeof(int) + (r * sizeof(struct Row));
            memcpy(&all_rows[row_index++], (char *)db->pages[p] + offset, sizeof(struct Row));
        }
    }

    // Clear all existing pages
    for (int p = 0; p < db->num_pages; p++)
    {
        memset(db->pages[p], 0, PAGE_SIZE);
        db->page_dirty[p] = 1;
    }

    // Redistribute rows to pages_needed pages
    row_index = 0;
    for (int p = 0; p < pages_needed; p++)
    {
        int rows_for_this_page = (p < total_rows % pages_needed) ?
                                (total_rows / pages_needed) + 1 :
                                (total_rows / pages_needed);

        // Set row count for this page
        memcpy(db->pages[p], &rows_for_this_page, sizeof(int));

        // Copy rows to this page
        for (int r = 0; r < rows_for_this_page; r++)
        {
            size_t offset = sizeof(int) + (r * sizeof(struct Row));
            memcpy((char *)db->pages[p] + offset, &all_rows[row_index++], sizeof(struct Row));
        }
    }

    // Free unused pages
    for (int p = pages_needed; p < db->num_pages; p++)
    {
        free(db->pages[p]);
        db->pages[p] = NULL;
    }

    db->num_pages = pages_needed;
    free(all_rows);
}

// Delete a row
int delete_row(Database *db, int id)
{
    if (id <= 0)
    {
        printf("Error: ID must be a positive integer (got %d)\n", id);
        return 0;
    }

    off_t address;
    btree_search(db, id, &address);
    if (address == -1)
    {
        printf("Error: Row with id=%d not found\n", id);
        return 0;
    }

    // Delete from B-Tree
    btree_delete(db, id);

    // Delete from data pages
    int found = 0;
    for (int page = 0; page < db->num_pages; page++)
    {
        int *page_num_rows = (int *)db->pages[page];
        for (int i = 0; i < *page_num_rows; i++)
        {
            size_t offset = sizeof(int) + (i * sizeof(struct Row));
            off_t computed_address = DATA_START_OFFSET + (off_t)page * PAGE_SIZE + offset;
            if (computed_address == address)
            {
                found = 1;
                // Shift all subsequent rows left to fill the gap
                for (int j = i; j < *page_num_rows - 1; j++)
                {
                    size_t current_offset = sizeof(int) + (j * sizeof(struct Row));
                    size_t next_offset = sizeof(int) + ((j + 1) * sizeof(struct Row));
                    memcpy((char *)db->pages[page] + current_offset,
                           (char *)db->pages[page] + next_offset,
                           sizeof(struct Row));
                }
                // Clear the last slot after shifting
                size_t last_offset = sizeof(int) + ((*page_num_rows - 1) * sizeof(struct Row));
                memset((char *)db->pages[page] + last_offset, 0, sizeof(struct Row));
                (*page_num_rows)--;

                int updated_num_rows = *page_num_rows;
                memcpy(db->pages[page], &updated_num_rows, sizeof(int));

                // handle empty pages
                if (*page_num_rows == 0)
                {
                    free(db->pages[page]);
                    for (int k = page; k < db->num_pages - 1; k++)
                    {
                        db->pages[k] = db->pages[k + 1];
                    }
                    db->pages[db->num_pages - 1] = NULL;
                    db->num_pages--;
                    if (db->num_pages == 0)
                    {
                        void *new_page = malloc(PAGE_SIZE);
                        if (new_page == NULL)
                        {
                            perror("Error: Could not allocate initial page\n");
                            free(db->pages);
                            fclose(db->file);
                            exit(1);
                        }
                        memset(new_page, 0, PAGE_SIZE);
                        db->pages[0] = new_page;
                        db->num_pages = 1;
                    }
                }
                db->page_dirty[page] = 1;
                break;
            }
        }
        if (found)
            break;
    }
    if (found)
    {
        // Compact pages after deletion
        compact_pages(db);
        write_buffer(db);
    }

    return found;
}
