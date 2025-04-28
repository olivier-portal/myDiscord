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

int create_channel(PGconn* conn, const char* name, int is_private) {
    const char* query = "INSERT INTO Channel (name_channel, is_private) VALUES ($1, $2)";
    char is_private_str[6];
    snprintf(is_private_str, sizeof(is_private_str), "%d", is_private);

    PGresult* res = PQexecParams(conn, query, 2, NULL, (const char*[]){name, is_private_str}, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "❌ Erreur lors de la création du canal: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return 0;
    }

    printf("✅ Canal créé avec succès.\n");
    PQclear(res);
    return 1;
}

int edit_channel(PGconn* conn, int channel_id, int user_id, const char* new_name, int is_private) {
    const char* query = 
        "UPDATE Channel SET name_channel = $1, is_private = $2 "
        "WHERE Id_channel = $3 AND EXISTS ("
        "    SELECT 1 FROM Channel_user WHERE Id_client = $4 AND Id_channel = $3 AND role = 'admin'"
        ")";
    char channel_id_str[12], user_id_str[12], is_private_str[6];
    snprintf(channel_id_str, sizeof(channel_id_str), "%d", channel_id);
    snprintf(user_id_str, sizeof(user_id_str), "%d", user_id);
    snprintf(is_private_str, sizeof(is_private_str), "%d", is_private);

    PGresult* res = PQexecParams(conn, query, 4, NULL, (const char*[]){new_name, is_private_str, channel_id_str, user_id_str}, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "❌ Erreur lors de la modification du canal: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return 0;
    }

    printf("✅ Canal modifié avec succès.\n");
    PQclear(res);
    return 1;
}

int select_user_channels(PGconn* conn, int user_id) {
    const char* query = 
        "SELECT c.Id_channel, c.name_channel, c.is_private, cu.role "
        "FROM Channel AS c "
        "JOIN Channel_user AS cu ON c.Id_channel = cu.Id_channel "
        "WHERE cu.Id_client = $1";
    char user_id_str[12];
    snprintf(user_id_str, sizeof(user_id_str), "%d", user_id);

    PGresult* res = PQexecParams(conn, query, 1, NULL, (const char*[]){user_id_str}, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "❌ Erreur lors de la récupération des canaux de l'utilisateur: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return 0;
    }

    printf("📋 Liste des canaux de l'utilisateur:\n");
    for (int i = 0; i < PQntuples(res); i++) {
        printf(" - [%s] %s (Privé: %s, Rôle: %s)\n",
               PQgetvalue(res, i, 0),
               PQgetvalue(res, i, 1),
               strcmp(PQgetvalue(res, i, 2), "t") == 0 ? "Oui" : "Non",
               PQgetvalue(res, i, 3));
    }

    PQclear(res);
    return 1;
}

int delete_channel(PGconn* conn, int channel_id, int user_id) {
    const char* query = 
        "DELETE FROM Channel WHERE Id_channel = $1 AND EXISTS ("
        "    SELECT 1 FROM Channel_user WHERE Id_client = $2 AND Id_channel = $1 AND role = 'admin'"
        ")";
    char channel_id_str[12], user_id_str[12];
    snprintf(channel_id_str, sizeof(channel_id_str), "%d", channel_id);
    snprintf(user_id_str, sizeof(user_id_str), "%d", user_id);

    PGresult* res = PQexecParams(conn, query, 2, NULL, (const char*[]){channel_id_str, user_id_str}, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "❌ Erreur lors de la suppression du canal: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return 0;
    }

    printf("✅ Canal supprimé avec succès.\n");
    PQclear(res);
    return 1;
}