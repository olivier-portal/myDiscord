#include "channel_crud.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int list_public_channels(PGconn* conn) {
    const char* query = "SELECT Id_channel, name_channel FROM Channel WHERE is_private = false";
    PGresult* res = PQexec(conn, query);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "❌ Erreur lors de la récupération des canaux publics: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return 0;
    }

    printf("📋 Liste des canaux publics:\n");
    for (int i = 0; i < PQntuples(res); i++) {
        printf(" - [%s] %s\n", PQgetvalue(res, i, 0), PQgetvalue(res, i, 1));
    }

    PQclear(res);
    return 1;
}