#include <gtk/gtk.h>
#include "theme.h"

void apply_custom_theme(GtkWidget *widget) {
    if (!widget) {
        g_printerr("❌ apply_custom_theme : widget NULL, impossible d'appliquer un thème.\n");
        return;
    }

    GdkDisplay *display = gtk_widget_get_display(widget);
    if (!GDK_IS_DISPLAY(display)) {
        g_printerr("❌ Erreur : affichage invalide (GDK_IS_DISPLAY failed).\n");
        return;
    }

    GtkCssProvider *provider = gtk_css_provider_new();

    // GTK Theme par défaut (MyBreeze-Dark)
    const gchar *theme_path = "C:/Users/porta/.themes/MyBreeze-Dark-GTK/gtk-4.0/gtk.css";
    GFile *theme_file = g_file_new_for_path(theme_path);

    GError *error = NULL;
    gtk_css_provider_load_from_file(provider, theme_file, &error);
    g_object_unref(theme_file);

    if (error) {
        g_printerr("❌ Échec chargement du thème : %s\n", error->message);
        g_clear_error(&error);
    } else {
        g_print("✅ Thème appliqué : %s\n", theme_path);
        gtk_style_context_add_provider_for_display(display,
            GTK_STYLE_PROVIDER(provider),
            GTK_STYLE_PROVIDER_PRIORITY_THEME);
    }

    // Ajout du custom.css optionnel (pas bloquant)
    const gchar *custom_path = "./custom.css";
    GFile *custom_file = g_file_new_for_path(custom_path);
    GtkCssProvider *custom = gtk_css_provider_new();

    gtk_css_provider_load_from_file(custom, custom_file, NULL);
    gtk_style_context_add_provider_for_display(display,
        GTK_STYLE_PROVIDER(custom),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_object_unref(custom_file);
}
