#include <gtk/gtk.h>
#include <libpq-fe.h>
#include <stdio.h>

static void on_activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "myDiscord - GTK4 + PostgreSQL");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 200);
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char *argv[]) {
    // Test PostgreSQL connection
    const char *conninfo = "dbname=postgres";
    PGconn *conn = PQconnectdb("host=localhost port=5432 dbname=postgres user=postgres password=monSuperMotDePasse");


    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Erreur PostgreSQL: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return 1;
    } else {
        printf("✅ Connexion PostgreSQL réussie.\n");
        PQfinish(conn);
    }

    // GTK 4 test
    GtkApplication *app = gtk_application_new("com.jinx.myDiscord", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
