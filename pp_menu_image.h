#ifndef PP_MENU_IMAGE_H
#define PP_MENU_IMAGE_H
#include <gtk/gtkwidget.h>
#include <gtk/gtkuimanager.h>
#include "layoutrectangle.h"
#include "pp_mainwindow.h"

void BuildImageMenu(void *userdata,GtkUIManager *ui_manager);
bool ImageMenu_GetCropFlag(GtkUIManager *ui_manager);
void ImageMenu_SetCropFlag(GtkUIManager *ui_manager,bool active);
bool ImageMenu_GetHFlipFlag(GtkUIManager *ui_manager);
void ImageMenu_SetHFlipFlag(GtkUIManager *ui_manager,bool active);
bool ImageMenu_GetVFlipFlag(GtkUIManager *ui_manager);
void ImageMenu_SetVFlipFlag(GtkUIManager *ui_manager,bool active);
enum PP_ROTATION ImageMenu_GetRotation(GtkUIManager *ui_manager);
void ImageMenu_SetRotation(GtkUIManager *ui_manager,enum PP_ROTATION rotation);
void ImageMenu_SetLayoutCapabilities(GtkUIManager *ui_manager,int features);
void ImageMenu_DoPopup(GtkUIManager *ui);
#endif
