#ifndef DB_CONNECTION_H
#define DB_CONNECTION_H

#include <libpq-fe.h>

// Structure to hold database configuration
typedef struct {
    char host[128];
    int port;
    char dbname[128];
    char user[128];
    char password[128];
} DBConfig;

// Load database configuration from a file
int load_db_config(const char *filepath, DBConfig *config);

// Connect to the database using a DBConfig structure
PGconn *connect_to_database(const DBConfig *config);

// Disconnect from the database
void disconnect_from_database(PGconn *conn);

#endif // DB_CONNECTION_H
