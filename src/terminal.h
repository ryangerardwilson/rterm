#ifndef TERMINAL_H
#define TERMINAL_H

#include <gtk/gtk.h>
#include <vte/vte.h>

#define MAX_TABS 8

typedef struct {
  GtkWidget *terminal;
  char *label;
  gpointer user_data;
} TerminalTab;

typedef struct {
  GList *tabs;
  TerminalTab *current;
  GtkWidget *window;
  GtkApplication *app;
  GtkWidget *radial_menu;
  int active_tab_index;
} AppData;

void init_app_data(AppData *app_data, GtkApplication *app, GtkWidget *window);
TerminalTab *create_terminal_tab(AppData *app_data);
void register_key_handlers(AppData *app_data,
                           GtkWidget *window); // Updated to include window
gboolean handle_app_key_press(GtkEventControllerKey *controller, guint keyval,
                              guint keycode, GdkModifierType state,
                              gpointer data);

#endif
