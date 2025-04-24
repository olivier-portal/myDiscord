#include <gtk/gtk.h>
#include "window.h"

void print_hello (GtkWidget *widget, gpointer data)
{
  g_print ("Hello World\n");
}

void apply_custom_theme() {
  GtkCssProvider *provider = gtk_css_provider_new();
  GdkDisplay *display = gdk_display_get_default();

  // Utiliser un chemin absolu compatible GTK (Windows ou Unix)
  #ifdef _WIN32
      gchar *theme_path = g_strdup("C:\\Users\\porta\\.themes\\MyBreeze-Dark-GTK\\gtk-4.0\\gtk.css");
  #else
      gchar *theme_path = g_build_filename(g_get_home_dir(), ".themes", "MyBreeze-Dark-GTK", "gtk-4.0", "gtk.css", NULL);
  #endif

  g_print("📂 Chargement du thème GTK depuis : [%s]\n", theme_path);

  if (!g_file_test(theme_path, G_FILE_TEST_EXISTS)) {
      g_printerr("❌ Le fichier thème est introuvable : [%s]\n", theme_path);
  } else {
      GFile *file = g_file_new_for_path(theme_path);
      gtk_css_provider_load_from_file(provider, file);
      g_object_unref(file);

      gtk_style_context_add_provider_for_display(display,
          GTK_STYLE_PROVIDER(provider),
          GTK_STYLE_PROVIDER_PRIORITY_USER);
  }

  g_free(theme_path);
}

void on_activate (GtkApplication *app, gpointer user_data)
{
  apply_custom_theme();

  GtkWidget *window;
  GtkWidget *grid;
  GtkWidget *button;

  /* create a new window, and set its title */
  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "myDiscord");
  // gtk_window_set_default_size (GTK_WINDOW (window), 1280, 800);

  /* Here we construct the container that is going pack our buttons */
  grid = gtk_grid_new ();

  /* Pack the container in the window */
  gtk_window_set_child (GTK_WINDOW (window), grid);

  button = gtk_button_new_with_label ("Button 1");
  g_signal_connect (button, "clicked", G_CALLBACK (print_hello), NULL);
  
  /* Place the first button in the grid cell (0, 0), and make it fill
   * just 1 cell horizontally and vertically (ie no spanning)
   */
  gtk_grid_attach (GTK_GRID (grid), button, 0, 0, 1, 1);

  button = gtk_button_new_with_label ("Button 2");
  g_signal_connect (button, "clicked", G_CALLBACK (print_hello), NULL);

  /* Place the second button in the grid cell (1, 0), and make it fill
   * just 1 cell horizontally and vertically (ie no spanning)
   */
  gtk_grid_attach (GTK_GRID (grid), button, 1, 0, 1, 1);

  button = gtk_button_new_with_label ("Quit");
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_window_destroy), window);

  /* Place the Quit button in the grid cell (0, 1), and make it
   * span 2 columns.
   */
  gtk_grid_attach (GTK_GRID (grid), button, 0, 1, 2, 1);

  gtk_window_present (GTK_WINDOW (window));
}
