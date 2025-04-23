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