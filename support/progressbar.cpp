/*
 * progressbar.cpp - A subclass of Progress; this provides a dialog with
 * a progress meter embedded within it, and optionally, a cancel button.
 *
 * Copyright (c) 2004-2006 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <cstdlib>
#include <cstring>
#include <iostream>

#include <gtk/gtk.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtkprogressbar.h>
#include <gtk/gtkstock.h>

#include "progressbar.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)

using namespace std;


void ProgressBar::cancel_callback(GtkWidget *wid,gpointer *ob)
{
	ProgressBar *p=(ProgressBar *)ob;
	p->cancelled=true;
}


ProgressBar::ProgressBar(const char *message,bool cancel,GtkWidget *parent)
	: Progress(), message(NULL), label(NULL), cancelled(false)
{
	if(message)
		this->message=strdup(message);
	window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	if(parent)
		gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(parent));
	gtk_window_set_title(GTK_WINDOW(window),_("Progress..."));
	gtk_window_set_default_size(GTK_WINDOW(window),150,30);
	gtk_widget_show(window);
	gtk_container_set_border_width(GTK_CONTAINER(window),10);

	GtkWidget *vbox=gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(window),vbox);
	gtk_widget_show(vbox);
	
	if(this->message)
	{
		label=gtk_label_new(this->message);
		gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_CENTER);
		gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,0);
		gtk_widget_show(label);
	}
	
	progressbar=gtk_progress_bar_new();
	gtk_box_pack_start(GTK_BOX(vbox),progressbar,FALSE,FALSE,0);
	gtk_widget_show(progressbar);

	if(cancel)	
	{
		GtkWidget *c=gtk_button_new_from_stock(GTK_STOCK_CANCEL);
		gtk_box_pack_start(GTK_BOX(vbox),c,FALSE,FALSE,0);
		gtk_widget_show(c);
		g_signal_connect(G_OBJECT(c),"clicked",G_CALLBACK(cancel_callback),this);
	}

	DoProgress(0,1);
}


ProgressBar::~ProgressBar()
{
	if(message)
		free(message);
	gtk_widget_destroy(window);
}


bool ProgressBar::DoProgress(int i,int maxi)
{
	Progress::DoProgress(i,maxi);
	if(maxi)
	{
		float v=i;
		v/=maxi;
		
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressbar),v);
	}
	else
		gtk_progress_bar_pulse(GTK_PROGRESS_BAR(progressbar));

	while(gtk_events_pending())
		gtk_main_iteration_do(false);

	if(cancelled)
		return(false);
	else
		return(true);
}


void ProgressBar::SetMessage(const char *msg)
{
	if(message)
		free(message);
	message=NULL;

	if(msg)
		message=strdup(msg);
	if(label)
		gtk_label_set_text(GTK_LABEL(label),message);
}
