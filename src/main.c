#include "terminal.h"
#include <gtk/gtk.h>

static void activate(GtkApplication *app, gpointer user_data) {
  GtkWidget *window = gtk_application_window_new(app);

  // Disable decorations and maximize the window
  gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
  gtk_window_maximize(GTK_WINDOW(window));

  // Set the window opacity to 70% (0.0 = fully transparent, 1.0 = fully opaque)
  gtk_widget_set_opacity(window, 0.7);

  // In GTK4, RGBA visuals and support for transparency are handled by the
  // compositor.

  AppData *app_data = g_new0(AppData, 1);
  init_app_data(app_data, app, window);

  gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char *argv[]) {
  GtkApplication *app = gtk_application_new("org.example.TerminalClone",
                                            G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}
