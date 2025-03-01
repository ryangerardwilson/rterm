cmake_minimum_required(VERSION 3.10)
project(GnomeTerminalClone C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)
pkg_check_modules(VTE REQUIRED vte-2.91-gtk4)
pkg_check_modules(GLIB REQUIRED glib-2.0)

include_directories(${GTK4_INCLUDE_DIRS} ${VTE_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS})
link_directories(${GTK4_LIBRARY_DIRS} ${VTE_LIBRARY_DIRS} ${GLIB_LIBRARY_DIRS})

add_executable(gnome-terminal-clone 
    src/main.c 
    src/terminal.c 
    src/radial_menu_ui.c 
    src/radial_menu_controller.c
    src/utils.c          # Added
)
target_link_libraries(gnome-terminal-clone ${GTK4_LIBRARIES} ${VTE_LIBRARIES} ${GLIB_LIBRARIES} m)
