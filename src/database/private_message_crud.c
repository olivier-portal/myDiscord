#include "private_message_crud.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int create_private_message(PGconn *conn, const PrivateMessage *private_message) {
    const char *query = "INSERT INTO Private_message (contenu, date_message, Id_sender, Id_target) VALUES ($1, NOW(), $2, $3)";
    char sender_id_str[12], target_id_str[12];
    snprintf(sender_id_str, sizeof(sender_id_str), "%d", private_message->sender_id);
    snprintf(target_id_str, sizeof(target_id_str), "%d", private_message->target_id);

    PGresult *res = PQexecParams(conn, query, 3, NULL, (const char*[]){private_message->content, sender_id_str, target_id_str}, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "❌ Erreur lors de l'envoi du message privé: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return 0;
    }

    printf("✅ Message privé envoyé avec succès.\n");
    PQclear(res);
    return 1;
}

PrivateMessage *select_private_messages(PGconn *conn, int user_id, int *private_message_count) {
    const char *query = "SELECT Id_private_message, contenu, date_message, Id_sender, Id_target "
                        "FROM Private_message WHERE Id_sender = $1 OR Id_target = $1 ORDER BY date_message ASC";
    char user_id_str[12];
    snprintf(user_id_str, sizeof(user_id_str), "%d", user_id);

    PGresult* res = PQexecParams(conn, query, 1, NULL, (const char*[]){user_id_str}, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "❌ Erreur lors de la récupération des messages privés: %s\n", PQerrorMessage(conn));
        PQclear(res);
        *private_message_count = 0;
        return NULL;
    }

    int rows = PQntuples(res);
    *private_message_count = rows;

    // Allocate memory for the array of PrivateMessage structs
    PrivateMessage *private_messages = (PrivateMessage*)malloc(rows *sizeof(PrivateMessage));
    if (!private_messages) {
        fprintf(stderr, "❌ Erreur d'allocation mémoire pour les messages privés.\n");
        PQclear(res);
        *private_message_count = 0;
        return NULL;
    }

    // Populate the array with data from the query resuy result
    for (int i = 0; i < rows; i++) {
        private_messages[i].id = atoi(PQgetvalue(res, i, 0));
        strncpy(private_messages[i].content, PQgetvalue(res, i, 1), sizeof(private_messages[i].content) - 1);
        strncpy(private_messages[i].date, PQgetvalue(res, i, 2), sizeof(private_messages[i].date) - 1);
        private_messages[i].sender_id = atoi(PQgetvalue(res, i, 3));
        private_messages[i].target_id = atoi(PQgetvalue(res, i, 4));
    }

    PQclear(res);
    return private_messages;
}

int update_private_message(PGconn *conn, int private_message_id, int sender_id, const char *new_content) {
    const char *query = "UPDATE Private_message SET content = $1, last_edit = NOW() WHERE Id_private_message = $2 AND Id_sender = $3";
    char private_message_id_str[12], sender_id_str[12];
    snprintf(private_message_id_str, sizeof(private_message_id_str), "%d", private_message_id);
    snprintf(sender_id_str, sizeof(sender_id_str), "%d", sender_id);

    PGresult *res = PQexecParams(conn, query, 3, NULL, (const char *[]){new_content, private_message_id_str, sender_id_str}, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "❌ Erreur lors de la modification du message privé: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return 0;
    }

    printf("✅ Message privé modifié avec succès.\n");
    PQclear(res);
    return 1;
}
