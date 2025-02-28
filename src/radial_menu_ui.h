#ifndef RADIAL_MENU_UI_H
#define RADIAL_MENU_UI_H

#include "terminal.h"
#include <gtk/gtk.h>

#define RADIUS 100
#define BUTTON_SIZE 50

void radial_menu_ui_setup(AppData *app_data);
void radial_menu_ui_toggle(AppData *app_data);
void radial_menu_ui_update(AppData *app_data);

#endif
