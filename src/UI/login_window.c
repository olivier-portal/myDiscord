#include <gtk/gtk.h>
#include "window.h"

// Callback pour ouvrir la fenêtre principale après login
static void on_login_clicked(GtkButton *button, gpointer app) {
    mydiscord_main_window(GTK_APPLICATION(app), NULL);
}

// Callback pour ouvrir une fenêtre d'inscription
static void on_create_account_clicked(GtkButton *button, gpointer user_data) {
    GtkWindow *parent = GTK_WINDOW(user_data);

    GtkWidget *dialog = gtk_dialog_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "Créer un compte");
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 300, 200);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_box_set_spacing(GTK_BOX(box), 10);
    gtk_widget_set_margin_top(box, 10);
    gtk_widget_set_margin_bottom(box, 10);
    gtk_widget_set_margin_start(box, 10);
    gtk_widget_set_margin_end(box, 10);

    GtkWidget *entry_user = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_user), "Nom d'utilisateur");
    GtkWidget *entry_pass = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_pass), "Mot de passe");
    gtk_entry_set_visibility(GTK_ENTRY(entry_pass), FALSE);

    GtkWidget *btn_register = gtk_button_new_with_label("S'inscrire");

    gtk_box_append(GTK_BOX(box), entry_user);
    gtk_box_append(GTK_BOX(box), entry_pass);
    gtk_box_append(GTK_BOX(box), btn_register);
    gtk_box_append(GTK_BOX(content_area), box);

    g_signal_connect_swapped(btn_register, "clicked", G_CALLBACK(gtk_window_destroy), dialog);

    gtk_window_present(GTK_WINDOW(dialog));
}

// Fenêtre de login principale
void login_window(GtkApplication *app) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Connexion");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 200);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_top(box, 20);
    gtk_widget_set_margin_bottom(box, 20);
    gtk_widget_set_margin_start(box, 30);
    gtk_widget_set_margin_end(box, 30);

    GtkWidget *entry_user = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_user), "Nom d'utilisateur");

    GtkWidget *entry_pass = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_pass), "Mot de passe");
    gtk_entry_set_visibility(GTK_ENTRY(entry_pass), FALSE);

    GtkWidget *btn_login = gtk_button_new_with_label("Se connecter");
    GtkWidget *btn_create = gtk_link_button_new_with_label("Créer un compte", "Créer un compte");

    gtk_box_append(GTK_BOX(box), entry_user);
    gtk_box_append(GTK_BOX(box), entry_pass);
    gtk_box_append(GTK_BOX(box), btn_login);
    gtk_box_append(GTK_BOX(box), btn_create);

    gtk_window_set_child(GTK_WINDOW(window), box);

    g_signal_connect(btn_login, "clicked", G_CALLBACK(on_login_clicked), app);
    g_signal_connect(btn_create, "clicked", G_CALLBACK(on_create_account_clicked), window);

    gtk_window_present(GTK_WINDOW(window));
}
