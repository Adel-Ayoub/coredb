#include "../include/coredb.h"

int main()
{
    Database db = init_db("coredb.db");
    run_repl(&db);
    close_db(&db);
    printf("File closed successfully\n");
    return 0;
}
