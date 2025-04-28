#include "channel_user_crud.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int add_user_to_channel(PGconn* conn, int client_id, int channel_id, const char* role) {
    const char* query = "INSERT INTO Channel_user (Id_client, Id_channel, role) VALUES ($1, $2, $3)";
    char client_id_str[12], channel_id_str[12];
    snprintf(client_id_str, sizeof(client_id_str), "%d", client_id);
    snprintf(channel_id_str, sizeof(channel_id_str), "%d", channel_id);

    PGresult* res = PQexecParams(conn, query, 3, NULL, (const char*[]){client_id_str, channel_id_str, role}, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "❌ Erreur lors de l'ajout de l'utilisateur au canal: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return 0;
    }

    printf("✅ Utilisateur ajouté au canal avec succès.\n");
    PQclear(res);
    return 1;
}

int remove_user_from_channel(PGconn* conn, int client_id, int channel_id) {
    const char* query = "DELETE FROM Channel_user WHERE Id_client = $1 AND Id_channel = $2";
    char client_id_str[12], channel_id_str[12];
    snprintf(client_id_str, sizeof(client_id_str), "%d", client_id);
    snprintf(channel_id_str, sizeof(channel_id_str), "%d", channel_id);

    PGresult* res = PQexecParams(conn, query, 2, NULL, (const char*[]){client_id_str, channel_id_str}, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "❌ Erreur lors de la suppression de l'utilisateur du canal: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return 0;
    }

    printf("✅ Utilisateur supprimé du canal avec succès.\n");
    PQclear(res);
    return 1;
}
