#ifndef RADIAL_MENU_CONTROLLER_H
#define RADIAL_MENU_CONTROLLER_H

#include "terminal.h"
#include <gtk/gtk.h>

void radial_menu_controller_init(AppData *app_data);
void radial_menu_controller_add_new_tab(AppData *app_data);
void radial_menu_controller_switch_to_tab(AppData *app_data, int index);
void radial_menu_controller_remove_current_tab(AppData *app_data);
gboolean radial_menu_controller_handle_key(GtkEventControllerKey *controller,
                                           guint keyval, guint keycode,
                                           GdkModifierType state,
                                           gpointer data);

#endif
