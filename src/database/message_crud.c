#include "message_crud.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int write_message(PGconn* conn, const Message* message) {
    const char* query = "INSERT INTO Message (contenu, date_message, Id_channel, Id_sender) VALUES ($1, NOW(), $2, $3)";
    char channel_id_str[12], sender_id_str[12];
    snprintf(channel_id_str, sizeof(channel_id_str), "%d", message->channel_id);
    snprintf(sender_id_str, sizeof(sender_id_str), "%d", message->sender_id);

    PGresult* res = PQexecParams(conn, query, 3, NULL, (const char*[]){message->content, channel_id_str, sender_id_str}, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "❌ Erreur lors de l'envoi du message: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return 0;
    }

    printf("✅ Message envoyé avec succès.\n");
    PQclear(res);
    return 1;
}

int edit_message(PGconn* conn, int message_id, int user_id, const char* new_content) {
    const char* query = "UPDATE Message SET content = $1, last_edit = NOW() WHERE Id_message = $2 AND Id_sender = $3";
    char message_id_str[12], user_id_str[12];
    snprintf(message_id_str, sizeof(message_id_str), "%d", message_id);
    snprintf(user_id_str, sizeof(user_id_str), "%d", user_id);

    PGresult* res = PQexecParams(conn, query, 3, NULL, (const char*[]){new_content, message_id_str, user_id_str}, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "❌ Erreur lors de la modification du message: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return 0;
    }

    printf("✅ Message modifié avec succès.\n");
    PQclear(res);
    return 1;
}

Message* fetch_channel_messages(PGconn* conn, int channel_id, int* message_count) {
    const char* query =
        "SELECT m.Id_message, c.name_channel, COALESCE(u.pseudo, 'Anonymous user'), m.date_message, m.content "
        "FROM message AS m "
        "JOIN channel AS c ON c.Id_channel = m.Id_channel "
        "LEFT JOIN client AS u ON u.Id_client = m.Id_sender "
        "WHERE c.is_private = FALSE AND c.Id_channel = $1 "
        "ORDER BY m.date_message ASC;";

    char channel_id_str[12];
    snprintf(channel_id_str, sizeof(channel_id_str), "%d", channel_id);

    PGresult* res = PQexecParams(conn, query, 1, NULL, (const char*[]){channel_id_str}, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "❌ Erreur lors de la récupération des messages: %s\n", PQerrorMessage(conn));
        PQclear(res);
        *message_count = 0;
        return NULL;
    }

    int rows = PQntuples(res);
    *message_count = rows;

    // Allocate memory for the array of Message structs
    Message* messages = (Message*)malloc(rows * sizeof(Message));
    if (!messages) {
        fprintf(stderr, "❌ Erreur d'allocation mémoire pour les messages.\n");
        PQclear(res);
        *message_count = 0;
        return NULL;
    }

    // Populate the array of 'messages' with data from the query result
    for (int i = 0; i < rows; i++) {
        strncpy(messages[i].content, PQgetvalue(res, i, 3), sizeof(messages[i].content) - 1);
        strncpy(messages[i].date, PQgetvalue(res, i, 2), sizeof(messages[i].date) - 1);
        messages[i].sender_id = -1; // Default value for sender_id (not fetched in this query)
        messages[i].channel_id = channel_id; // Set the channel ID
    }

    PQclear(res);
    return messages;
}