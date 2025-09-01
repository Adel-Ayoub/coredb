#include "../../include/coredb.h"

// Read a page from file into memory
int read_page_from_file(Database *db, int page_num)
{
    if (page_num < 0 || page_num >= db->num_pages)
    {
        return 0;
    }
    
    fseek(db->file, DATA_START_OFFSET + (off_t)page_num * PAGE_SIZE, SEEK_SET);
    size_t bytes_read = fread(db->pages[page_num], 1, PAGE_SIZE, db->file);
    return (bytes_read == PAGE_SIZE);
}

// Write a page from memory to file
int write_page_to_file(Database *db, int page_num)
{
    if (page_num < 0 || page_num >= db->num_pages)
    {
        return 0;
    }
    
    fseek(db->file, DATA_START_OFFSET + (off_t)page_num * PAGE_SIZE, SEEK_SET);
    size_t bytes_written = fwrite(db->pages[page_num], 1, PAGE_SIZE, db->file);
    return (bytes_written == PAGE_SIZE);
}

// Flush all dirty pages to disk
void flush_all_pages(Database *db)
{
    for (int i = 0; i < db->num_pages; i++)
    {
        if (db->page_dirty[i])
        {
            write_page_to_file(db, i);
            db->page_dirty[i] = 0;
        }
    }
}
