#include "db_connection.h"

int load_db_config(const char* filepath, DBConfig* config) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        perror("Erreur ouverture db_config.txt");
        return 0;
    }

    char line[128];
    while (fgets(line, sizeof(line), file)) {
        // Delete trailing newline characters
        line[strcspn(line, "\r\n")] = 0;

        // Ignore empty lines and comments
        if (line[0] == '\0' || line[0] == '#') continue;

        // Parse key-value pairs
        char* key = strtok(line, "=");
        char* value = strtok(NULL, "=");

        if (strcmp(key, "host") == 0) {
            strncpy(config->host, value, sizeof(config->host) - 1);
        } else if (strcmp(key, "port") == 0) {
            config->port = atoi(value);
        } else if (strcmp(key, "dbname") == 0) {
            strncpy(config->dbname, value, sizeof(config->dbname) - 1);
        } else if (strcmp(key, "user") == 0) {
            strncpy(config->user, value, sizeof(config->user) - 1);
        } else if (strcmp(key, "password") == 0) {
            strncpy(config->password, value, sizeof(config->password) - 1);
        }
    }

    fclose(file);
    return 1;
}

PGconn* connect_to_database(const DBConfig* config) {
    char conninfo[512];
    snprintf(conninfo, sizeof(conninfo),
             "host=%s port=%d dbname=%s user=%s password=%s",
             config->host, config->port, config->dbname, config->user, config->password);

    PGconn *conn = PQconnectdb(conninfo);

    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "❌ Erreur de connexion à la base de données: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return NULL;
    }

    printf("✅ Connexion à la base de données réussie.\n");
    return conn;
}

void disconnect_from_database(PGconn* conn) {
    if (conn) {
        PQfinish(conn);
        printf("✅ Déconnexion de la base de données réussie.\n");
    }
}
