#include "client_crud.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int create_client_account(PGconn* conn, const Client* client) {
    const char* query = "INSERT INTO Client (first_name, last_name, pseudo, password, email) VALUES ($1, $2, $3, $4, $5)";
    PGresult* res = PQexecParams(conn, query, 5, NULL, (const char*[]){client->name, client->last_name, client->pseudo, client->password, client->email}, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "❌ Erreur lors de la création du compte client: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return 0;
    }

    printf("✅ Compte client créé avec succès.\n");
    PQclear(res);
    return 1;
}

int authenticate_client(PGconn* conn, const char* pseudo, const char* password) {
    const char* query = "SELECT COUNT(*) FROM Client WHERE pseudo = $1 AND password = $2";
    PGresult* res = PQexecParams(conn, query, 2, NULL, (const char*[]){pseudo, password}, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "❌ Erreur lors de l'authentification: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return 0;
    }

    int authenticated = atoi(PQgetvalue(res, 0, 0)) > 0;
    PQclear(res);

    if (authenticated) {
        printf("✅ Authentification réussie.\n");
    } else {
        printf("❌ Authentification échouée.\n");
    }

    return authenticated;
}