#ifndef PHOTOPRINT_DIALOGS_H
#define PHOTOPRINT_DIALOGS_H

#include <string>
#include <gtk/gtkwindow.h>

#include "photoprint_state.h"

void ColourManagement_Dialog(GtkWindow *parent,PhotoPrint_State &state);
void ColourResponseTag_Dialog(GtkWidget *parent,PhotoPrint_State &state);
void Units_Dialog(GtkWindow *parent,PhotoPrint_State &state);
void Scaling_Dialog(GtkWindow *parent,PhotoPrint_State &state);
void RenderingResolution_Dialog(GtkWindow *parent,PhotoPrint_State &state);
void PrintSetup_Dialog(GtkWindow *parent,PhotoPrint_State &state);
void Paths_Dialog(GtkWindow *parent,PhotoPrint_State &state);
std::string ImageMask_Dialog(GtkWindow *parent,PhotoPrint_State &state,std::string oldfn);
std::string Background_Dialog(GtkWindow *parent,PhotoPrint_State &state,std::string oldfn);
void SetCustomProfileDialog(GtkWindow *parent,PhotoPrint_State &state,Layout_ImageInfo *ii);

void ExportTiff_Dialog(GtkWindow *parent,PhotoPrint_State &state);
void ExportJPEG_Dialog(GtkWindow *parent,PhotoPrint_State &state);

void About_Dialog(GtkWindow *parent);
#endif
