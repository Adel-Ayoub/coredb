#include "../../include/coredb.h"

// REPL loop (unchanged)
void run_repl(Database *db)
{
    // print instructions
    printf("Welcome to the database REPL!\n");
    printf("Available Commands:\n");
    printf("  INSERT <id> <name>      - Insert a new row\n");
    printf("  SELECT <id>             - Select a row by ID\n");
    printf("  SELECT                  - Select all rows\n");
    printf("  UPDATE <id> <new_name>  - Update a row by ID\n");
    printf("  DELETE <id>             - Delete a row by ID\n");
    printf("  exit                    - Exit the REPL\n");
    char input[100];
    while (1)
    {
        printf("db>");
        fflush(stdout);
        // Read part of REPL loop --------
        if (fgets(input, sizeof(input), stdin) == NULL)
            break;                       // EOF or error
        input[strcspn(input, "\n")] = 0; // Remove newline character

        // Evaluate & Print part of REPL loop --------
        if (strncmp(input, "INSERT", 6) == 0)
        {
            int id;
            char name[60];
            if (sscanf(input, "INSERT %d %59s", &id, name) != 2)
            {
                printf("Error: Invalid INSERT format. Use: INSERT <id> <name>\n");
                continue;
            }
            if (id <= 0)
            {
                printf("Error: ID must be a positive integer (got %d)\n", id);
                continue;
            }

            if (insert_row(db, id, name))
            {
                printf("Inserted row: id=%d, name=%s\n", id, name);
            }
        }
        else if (strncmp(input, "SELECT", 6) == 0)
        {
            int id;
            char trailing[100];
            if (sscanf(input, "SELECT %d %s", &id, trailing) == 2)
            {
                printf("Error: Invalid SELECT format. Use: SELECT <id> or SELECT\n");
                continue;
            }
            if (sscanf(input, "SELECT %d", &id) == 1)
            {
                if (id <= 0)
                {
                    printf("Error: ID must be a positive integer (got %d)\n", id);
                    continue;
                }
                struct Row row;
                if (select_by_id(db, id, &row))
                {
                    printf("Row: id=%d, name=%s\n", row.id, row.name);
                }
                else
                {
                    printf("Row with id=%d not found\n", id);
                }
            }
            else
            {
                struct Row rows[MAX_ROWS];
                int count = select_rows(db, rows, MAX_ROWS);
                if (count == 0)
                {
                    printf("No rows to display\n");
                }
                else
                {
                    for (int i = 0; i < count; i++)
                    {
                        printf("Row %d: id=%d, name=%s\n", i, rows[i].id, rows[i].name);
                    }
                }
            }
        }
        else if (strncmp(input, "UPDATE", 6) == 0)
        {
            int id;
            char name[60];
            if (sscanf(input, "UPDATE %d %59s", &id, name) != 2)
            {
                printf("Error: Invalid UPDATE format. Use: UPDATE <id> <new_name>\n");
                continue;
            }
            if (id <= 0)
            {
                printf("Error: ID must be a positive integer (got %d)\n", id);
                continue;
            }
            if (update_row(db, id, name))
            {
                printf("Updated row: id=%d, new name=%s\n", id, name);
            }
        }
        else if (strncmp(input, "DELETE", 6) == 0)
        {
            int id;
            if (sscanf(input, "DELETE %d", &id) != 1)
            {
                printf("Error: Invalid DELETE format. Use: DELETE <id>\n");
                continue;
            }
            if (id <= 0)
            {
                printf("Error: ID must be a positive integer (got %d)\n", id);
                continue;
            }
            if (!delete_row(db, id))
            {
                printf("Row with id=%d not found\n", id);
            }
            else
            {
                printf("Deleted row with id=%d\n", id);
            }
        }
        else if (strncmp(input, "exit", 4) == 0)
        {
            break; // Exit the loop
        }
        else
        {
            printf("You entered: %s\n", input);
        }
    }
}
