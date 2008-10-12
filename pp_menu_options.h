#ifndef PP_MENU_OPTIONS_H
#define PP_MENU_OPTIONS_H
#include <gtk/gtkwidget.h>
#include <gtk/gtkuimanager.h>

#include "profilemanager/profilemanager.h"

void BuildOptionsMenu(void *userdata,GtkUIManager *ui_manager);
void OptionsMenu_SetProofMode(GtkUIManager *ui_manager,enum CMProofMode item);
void OptionsMenu_SetHighresPreviews(GtkUIManager *ui_manager,int hrpreview);
#endif
