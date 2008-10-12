/*
 * generaldialogs.cpp - routines to handle general-purpose dialogs - namely:
 * Error message dialog
 * File dialog (returns a char *filename to be freed by the app)
 *
 * Copyright (c) 2004-2006 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */


#include <iostream>

#include <sys/stat.h>

#include <gtk/gtk.h>

#include "egg-pixbuf-thumbnail.h"

#include "generaldialogs.h"
#include "searchpath.h"

using namespace std;


// Error message handling


void ErrorMessage_Dialog(const char *message,GtkWidget *parent)
{
	GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW(parent),GtkDialogFlags(0),
		GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,
		message);
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
}


// File dialog

static void updatepreview(GtkWidget *fc,void *ud)
{
	GtkWidget *preview=GTK_WIDGET(ud);
	gchar *fn=gtk_file_chooser_get_preview_filename(GTK_FILE_CHOOSER(fc));
	bool active=false;
	if(fn)
	{
		GError *err=NULL;
		GdkPixbuf *pb=egg_pixbuf_get_thumbnail_for_file(fn,EGG_PIXBUF_THUMBNAIL_NORMAL,&err);
		if(pb)
		{
			gtk_image_set_from_pixbuf(GTK_IMAGE(preview),pb);
			g_object_unref(pb);
			active=true;		
		}	
	}
	gtk_file_chooser_set_preview_widget_active(GTK_FILE_CHOOSER(fc),active);
}


char *File_Dialog(const char *title,const char *oldfilename,GtkWidget *parent,bool preview)
{

#if 0
	char *newfile=NULL;
	GtkWidget *sel=gtk_file_selection_new(title);
	if(oldfilename)
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(sel),oldfilename);
	
	gint result=gtk_dialog_run(GTK_DIALOG(sel));
	if(result==GTK_RESPONSE_OK)
		newfile=g_strdup(gtk_file_selection_get_filename(GTK_FILE_SELECTION(sel)));

	gtk_widget_destroy(GTK_WIDGET(sel));
	
	return(newfile);

#else

	char *newfile=NULL;
	GtkWidget *dialog;

	dialog = gtk_file_chooser_dialog_new (title,
		GTK_WINDOW(parent),
		GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
		NULL);

	if(oldfilename)
	{
//		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),g_path_get_dirname(oldfilename));
		gtk_file_chooser_select_filename(GTK_FILE_CHOOSER(dialog),oldfilename);
	}

	if(preview)
	{
		GtkWidget *preview=gtk_image_new();
		gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(dialog),GTK_WIDGET(preview));
		g_signal_connect(G_OBJECT(dialog),"selection-changed",G_CALLBACK(updatepreview),preview);
	}
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
		newfile = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));


	gtk_widget_destroy (dialog);

	return(newfile);

#endif

}


char *File_Save_Dialog(const char *title,const char *oldfilename,GtkWidget *parent)
{

#if 0
	char *newfile=NULL;
	GtkWidget *sel=gtk_file_selection_new(title);
	if(oldfilename)
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(sel),oldfilename);
	
	gint result=gtk_dialog_run(GTK_DIALOG(sel));
	if(result==GTK_RESPONSE_OK)
		newfile=g_strdup(gtk_file_selection_get_filename(GTK_FILE_SELECTION(sel)));

	gtk_widget_destroy(GTK_WIDGET(sel));
	
	return(newfile);

#else

	char *newfile=NULL;
	GtkWidget *dialog;

	dialog = gtk_file_chooser_dialog_new (title,
		GTK_WINDOW(parent),
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
		NULL);

	if(oldfilename)
		cerr << "Setting old filename to: " << oldfilename << endl;
	if(oldfilename)
	{
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),g_path_get_dirname(oldfilename));
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog),g_path_get_basename(oldfilename));
	}

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		newfile = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

		cerr << "Checking if file " << newfile << " already exists?" << endl;
		struct stat statbuf;
		if(stat(newfile,&statbuf)==0)
		{
			cerr << "Yes - asking for confirmation before overwriting..." << endl;
			GtkWidget *confirm = gtk_message_dialog_new (GTK_WINDOW(parent),GtkDialogFlags(0),
			GTK_MESSAGE_WARNING,GTK_BUTTONS_OK_CANCEL,
			"File exists - OK to overwrite?");
			gint response=gtk_dialog_run (GTK_DIALOG (confirm));
			cerr << "Response: " << response << endl;
			if(response!=GTK_RESPONSE_OK)
			{
				g_free(newfile);
				newfile=NULL;
			}
			gtk_widget_destroy (confirm);
		}
	}

	gtk_widget_destroy (dialog);

	return(newfile);

#endif

}


char *Directory_Dialog(const char *title,const char *oldfilename,GtkWidget *parent)
{
	char *dirname=NULL;
	GtkWidget *dialog;

	dialog = gtk_file_chooser_dialog_new (title,
		GTK_WINDOW(parent),
		GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
		NULL);

	if(oldfilename)
	{
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),g_path_get_dirname(oldfilename));
	}

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
		dirname = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

	gtk_widget_destroy (dialog);

	return(dirname);
}

