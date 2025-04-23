#include "reaction_crud.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int add_reaction(PGconn* conn, const Reaction* reaction) {
    const char* query = "INSERT INTO Reaction (emoji, Id_client, Id_message) VALUES ($1, $2, $3)";
    char client_id_str[12], message_id_str[12];
    snprintf(client_id_str, sizeof(client_id_str), "%d", reaction->client_id);
    snprintf(message_id_str, sizeof(message_id_str), "%d", reaction->message_id);

    PGresult* res = PQexecParams(conn, query, 3, NULL, (const char*[]){reaction->emoji, client_id_str, message_id_str}, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "❌ Erreur lors de l'ajout de la réaction: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return 0;
    }

    printf("✅ Réaction ajoutée avec succès.\n");
    PQclear(res);
    return 1;
}