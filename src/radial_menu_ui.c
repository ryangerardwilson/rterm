#include "radial_menu_ui.h"
#include "radial_menu_controller.h"
#include "utils.h"
#include <math.h>
#include <stdio.h>

void radial_menu_ui_setup(AppData *app_data) {
    app_data->radial_menu = gtk_drawing_area_new();
    gtk_widget_set_size_request(app_data->radial_menu, RADIUS * 2 + BUTTON_SIZE + 20, RADIUS * 2 + BUTTON_SIZE + 20);
    gtk_widget_set_halign(app_data->radial_menu, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(app_data->radial_menu, GTK_ALIGN_CENTER);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(app_data->radial_menu),
                                   utils_radial_menu_ui_setup_draw_radial_menu, app_data, NULL);

    gtk_overlay_add_overlay(GTK_OVERLAY(app_data->window), app_data->radial_menu);
    gtk_widget_set_visible(app_data->radial_menu, FALSE);

    GtkGesture *click_gesture = gtk_gesture_click_new();
    g_signal_connect(click_gesture, "pressed", G_CALLBACK(utils_radial_menu_ui_setup_radial_menu_clicked), app_data);
    gtk_widget_add_controller(app_data->radial_menu, GTK_EVENT_CONTROLLER(click_gesture));

    printf("Radial menu initialized with %d tabs...\n", g_list_length(app_data->tabs));
}

void radial_menu_ui_toggle(AppData *app_data) {
    if (!app_data->radial_menu) {
        radial_menu_ui_setup(app_data);
        return;
    }
    gboolean visible = gtk_widget_get_visible(app_data->radial_menu);
    gtk_widget_set_visible(app_data->radial_menu, !visible);
    printf("Ctrl+T detected, %s radial menu...\n", visible ? "hiding" : "showing");
    if (!visible) {
        printf("Showing radial menu with %d tabs...\n", g_list_length(app_data->tabs));
        gtk_widget_queue_draw(app_data->radial_menu);
    }
}

void radial_menu_ui_update(AppData *app_data) {
    if (app_data->radial_menu) {
        gtk_widget_queue_draw(app_data->radial_menu);
    }
}
