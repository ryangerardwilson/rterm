#ifndef UTILS_H
#define UTILS_H

#include <gtk/gtk.h>
#include <vte/vte.h>
#include "terminal.h"

// Existing declarations
void utils_main_activate(GtkApplication *app, gpointer user_data);
void utils_init_app_data_apply_css(GtkWidget *window, GtkWidget *overlay);
void utils_init_app_data_log_display_info(void);
void utils_handle_app_key_press_log_key_event(guint keyval, guint keycode, GdkModifierType state);
void utils_radial_menu_ui_setup_draw_radial_menu(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer data);
void utils_radial_menu_ui_setup_radial_menu_clicked(GtkGesture *gesture, int n_press, gdouble x, gdouble y, gpointer data);
void utils_spawn_callback(VteTerminal *terminal, GPid pid, GError *error, gpointer user_data);
gboolean utils_update_label_after_idle(gpointer user_data);

#endif
