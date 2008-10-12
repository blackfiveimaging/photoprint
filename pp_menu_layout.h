#ifndef PP_MENU_LAYOUT_H
#define PP_MENU_LAYOUT_H
#include <gtk/gtkwidget.h>
#include <gtk/gtkuimanager.h>

void BuildLayoutMenu(void *userdata,GtkUIManager *ui_manager);
void LayoutMenu_SetLayout(GtkUIManager *ui_manager,const char *layouttype);
void LayoutMenu_SetLayoutCapabilities(GtkUIManager *ui_manager,int features);

#endif
