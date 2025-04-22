#include <gtk/gtk.h>
#include "config/config.h"
#include "server/server_socket.h"
#include "client/client_socket.h"
#include "ui/window.h"

int main(int argc, char *argv[]) {
    char *conninfo = load_conninfo_from_file("config.txt");
    if (!conninfo) {
        g_printerr("❌ config.txt manquant ou illisible\n");
        return 1;
    }

    start_server();
    start_client();

    GtkApplication *app = gtk_application_new("com.jinx.myDiscord", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}