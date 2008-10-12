#ifndef GENERAL_DIALOGS_H
#define GENERAL_DIALOGS_H

#include <gtk/gtkwindow.h>

void ErrorMessage_Dialog(const char *message,GtkWidget *parent=NULL);
char *File_Dialog(const char *title,const char *oldfilename,GtkWidget *parent=NULL,bool preview=false);
char *File_Save_Dialog(const char *title,const char *oldfilename,GtkWidget *parent=NULL);
char *Directory_Dialog(const char *title,const char *oldfilename,GtkWidget *parent=NULL);
#endif
