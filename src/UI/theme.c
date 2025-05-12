#include <gtk/gtk.h>
#include "theme.h"

void apply_custom_theme() {
    GdkDisplay *display = gdk_display_get_default();

    if (!GDK_IS_DISPLAY(display)) {
        g_printerr("❌ Display non valide ! Thème non appliqué.\n");
        return;
    }

    GtkCssProvider *provider = gtk_css_provider_new();
    GtkCssProvider *custom = gtk_css_provider_new();

    // 🎨 Thème MyBreeze
#ifdef _WIN32
    gchar *theme_path = g_strdup("C:\\Users\\porta\\.themes\\MyBreeze-Dark-GTK\\gtk-4.0\\gtk.css");
#else
    gchar *theme_path = g_build_filename(g_get_home_dir(), ".themes", "MyBreeze-Dark-GTK", "gtk-4.0", "gtk.css", NULL);
#endif

    GFile *theme_file = g_file_new_for_path(theme_path);
    gtk_css_provider_load_from_file(provider, theme_file);
    g_object_unref(theme_file);

    // 🎨 Custom CSS
    gchar *css_path = g_build_filename(g_get_current_dir(), "custom.css", NULL);
    GFile *custom_file = g_file_new_for_path(css_path);
    gtk_css_provider_load_from_file(custom, custom_file);
    g_object_unref(custom_file);

    // 📦 Application globale
    gtk_style_context_add_provider_for_display(display, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_THEME);
    gtk_style_context_add_provider_for_display(display, GTK_STYLE_PROVIDER(custom), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_free(theme_path);
    g_free(css_path);
}
