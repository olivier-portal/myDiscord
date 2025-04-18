#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <libpq-fe.h>
#include <direct.h> // Use this for _chdir on Windows

// 🔧 Fonction pour lire config.txt
char* load_conninfo_from_file(const char* filepath) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        perror("Erreur ouverture config.txt");
        return NULL;
    }

    static char conninfo[512] = "";
    char line[128];

    while (fgets(line, sizeof(line), file)) {
        // Supprimer le saut de ligne
        line[strcspn(line, "\r\n")] = 0;

        // Ignorer lignes vides ou commentaires
        if (line[0] == '\0' || line[0] == '#') continue;

        strcat(conninfo, line);
        strcat(conninfo, " ");
    }

    fclose(file);
    return conninfo;
}

// 🖼️ Fonction appelée à l'ouverture de la fenêtre GTK
static void on_activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "myDiscord - GTK4 + PostgreSQL");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 200);
    gtk_widget_set_visible(window, TRUE); // Replaces gtk_widget_show
}

int main(int argc, char *argv[]) {
    printf("🟡 Changement du répertoire de travail...\n");
    if (_chdir("d:/_Thibault/Documents/_Plateforme/!Dev_log_1/MyDiscord/myDiscord") != 0) { // Use _chdir for Windows
        perror("❌ Impossible de changer le répertoire de travail");
        system("pause");
        return 1;
    }

    printf("🟡 Lecture config...\n");
    char *conninfo = load_conninfo_from_file("config.txt"); // Use the correct relative path
    if (!conninfo) {
        fprintf(stderr, "❌ config.txt manquant ou illisible\n");
        system("pause");
        return 1;
    }

    printf("🟡 Connexion à PostgreSQL...\n");
    PGconn *conn = PQconnectdb(conninfo);
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "❌ Erreur PostgreSQL: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        system("pause");
        return 1;
    }

    printf("✅ Connexion PostgreSQL réussie.\n");
    PQfinish(conn);

    printf("🟢 Lancement de GTK...\n");
    GtkApplication *app = gtk_application_new("com.jinx.myDiscord", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    printf("🏁 Fin du programme (GTK fermée).\n");
    return status;
}
