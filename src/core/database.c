#include "../../include/coredb.h"
#include <unistd.h>

// Initialize the database
Database init_db(const char *filename)
{
    Database db;
    db.file = fopen(filename, "r+");
    if (db.file == NULL)
    {
        db.file = fopen(filename, "w+");
        if (db.file == NULL)
        {
            perror("Error: Could not create file\n");
            exit(1);
        }
        fclose(db.file);
        db.file = fopen(filename, "r+");
        if (db.file == NULL)
        {
            perror("Error: Could not reopen file\n");
            exit(1);
        }
        // Initialize B-Tree with an empty root node
        db.root_offset = PAGE_SIZE; // root at start of first index page
        BTreeNode root = {0};
        root.is_leaf = 1;
        write_node(&db, db.root_offset, &root);
        fseek(db.file, 0, SEEK_SET);
        fwrite(&db.root_offset, sizeof(off_t), 1, db.file);
    }
    else
    {
        // Read root_offset
        fseek(db.file, 0, SEEK_SET);
        fread(&db.root_offset, sizeof(off_t), 1, db.file);
    }
    db.max_pages = MAX_PAGES;
    db.pages = malloc(db.max_pages * sizeof(void *)); // 10 * 8 bytes
    if (db.pages == NULL)
    {
        perror("Error: Could not allocate pages array\n");
        fclose(db.file);
        exit(1);
    }
    db.num_pages = 0;
    for (int i = 0; i < MAX_PAGES; i++)
    {
        db.page_dirty[i] = 0;
    }

    // read data pages
    fseek(db.file, DATA_START_OFFSET, SEEK_SET);
    void *temp_buffer = malloc(PAGE_SIZE);
    if (temp_buffer == NULL)
    {
        perror("Error: Could not allocate temp buffer\n");
        free(db.pages);
        fclose(db.file);
        exit(1);
    }

    while (1)
    {
        size_t bytesRead = fread(temp_buffer, 1, PAGE_SIZE, db.file);
        if (bytesRead == 0)
            break;
        if (bytesRead < PAGE_SIZE && !feof(db.file))
        {
            printf("Error: Partial read, only %zu bytes read\n", bytesRead);
            free(temp_buffer);
            free(db.pages);
            fclose(db.file);
            exit(1);
        }
        void *page = malloc(PAGE_SIZE);
        if (page == NULL)
        {
            perror("Error: Could not allocate page\n");
            free(temp_buffer);
            free(db.pages);
            fclose(db.file);
            exit(1);
        }
        memcpy(page, temp_buffer, PAGE_SIZE);
        db.pages[db.num_pages] = page;
        db.num_pages++;

        if (db.num_pages >= db.max_pages)
        {
            printf("Warning: Maximum pages reached\n");
            break;
        }
    }
    free(temp_buffer);

    if (db.num_pages == 0)
    {
        void *page = malloc(PAGE_SIZE);
        if (page == NULL)
        {
            perror("Error: Could not allocate first page\n");
            free(db.pages);
            fclose(db.file);
            exit(1);
        }
        memset(page, 0, PAGE_SIZE); // Initialize the first page to zero
        db.pages[0] = page;
        db.num_pages = 1;
    }
    return db;
}

// Write the buffer to the disk file
void write_buffer(Database *db)
{
    // Write root_offset
    fseek(db->file, 0, SEEK_SET);
    fwrite(&db->root_offset, sizeof(off_t), 1, db->file);

    // Write data pages
    for (int i = 0; i < db->num_pages; i++)
    {
        fseek(db->file, DATA_START_OFFSET + (off_t)i * PAGE_SIZE, SEEK_SET);
        size_t bytesWritten = fwrite(db->pages[i], 1, PAGE_SIZE, db->file);
        if (bytesWritten != PAGE_SIZE)
        {
            printf("Error: Failed to write page %d, wrote %zu bytes\n", i, bytesWritten);
            exit(1);
        }
        db->page_dirty[i] = 0; // Reset dirty flag after writing
    }

    // Truncate file to remove any unused pages at the end
    off_t new_file_size = DATA_START_OFFSET + (off_t)db->num_pages * PAGE_SIZE;
    if (ftruncate(fileno(db->file), new_file_size) != 0)
    {
        perror("Warning: Could not truncate file");
        // Don't exit here, just continue - this is not a critical error
    }

    fflush(db->file); // ensure data is written to disk
}

// cleanup function
void close_db(Database *db)
{
    for (int i = 0; i < db->num_pages; i++)
    {
        free(db->pages[i]);
    }
    free(db->pages);
    fclose(db->file);
}
