
/*
 * dialogs.cpp - routines to handle PhotoPrint's dialogs - namely:
 * Print setup
 * Colour Management setup
 *
 * Copyright (c) 2004-2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */


#include <iostream>
#include <string>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "miscwidgets/generaldialogs.h"
#include "miscwidgets/pixbufview.h"
#include "miscwidgets/simplecombo.h"

#include "support/debug.h"
#include "support/progressbar.h"
#include "support/util.h"

#include "imagesource/pixbuf_from_imagesource.h"


#include "config.h"
#include "gettext.h"
#define _(x) gettext(x)

#include "dialogs.h"

using namespace std;


class printpreviewprogress : public Progress
{
	public:
	printpreviewprogress(const char *message,GtkWidget *parent)
	: Progress(), message(NULL), label(NULL)
	{
		if(message)
			this->message=strdup(message);
		window=gtk_window_new(GTK_WINDOW_POPUP);
		if(parent)
			gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(parent));
		gtk_window_set_default_size(GTK_WINDOW(window),150,30);
		gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER_ON_PARENT);
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

		DoProgress(0,1);
	}
	~printpreviewprogress()
	{
		if(message)
			free(message);
		gtk_widget_destroy(window);
	}
	bool DoProgress(int i,int maxi)
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
		return(true);
	}
	void SetMessage(const char *msg)
	{
		if(message)
			free(message);
		message=NULL;

		if(msg)
			message=strdup(msg);
		if(label)
			gtk_label_set_text(GTK_LABEL(label),message);
	}
	protected:
	GtkWidget *window;
	char *message;
	GtkWidget *label;
	GtkWidget *progressbar;
};


class printpreviewdata
{
	public:
	printpreviewdata(PhotoPrint_State &state) : state(state)
	{
		factory=state.profilemanager.GetTransformFactory();
		window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_default_size(GTK_WINDOW(window),800,600);
		gtk_widget_set_name(GTK_WIDGET(window),"PrintPreview");
		gtk_window_fullscreen(GTK_WINDOW(window));
		g_signal_connect(G_OBJECT(window),"delete-event",G_CALLBACK(deleteevent),this);

		pview=pixbufview_new(NULL,false);
		gtk_container_add(GTK_CONTAINER(window),pview);
		gtk_widget_show_all(window);

		popupshown=false;
		popup=gtk_window_new(GTK_WINDOW_POPUP);
		gtk_window_set_transient_for(GTK_WINDOW(popup),GTK_WINDOW(window));
		GtkWidget *vbox=gtk_vbox_new(FALSE,0);
		gtk_container_set_border_width(GTK_CONTAINER(popup),8);
		gtk_container_add(GTK_CONTAINER(popup),vbox);
		g_signal_connect(G_OBJECT(popup),"delete-event",G_CALLBACK(deleteevent),this);

		GtkWidget *tmp;

		GtkWidget *hbox=gtk_hbox_new(FALSE,0);
		gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);

		tmp=gtk_label_new(_("Page:"));
		gtk_box_pack_start(GTK_BOX(hbox),tmp,FALSE,FALSE,0);

		pagewidget=gtk_spin_button_new_with_range(1,state.layout->GetPages(),1);
		g_signal_connect(G_OBJECT(pagewidget),"value-changed",G_CALLBACK(page_changed),this);
		gtk_box_pack_start(GTK_BOX(hbox),pagewidget,FALSE,FALSE,8);

		tmp=gtk_label_new(_("Preview resolution:"));
		gtk_box_pack_start(GTK_BOX(hbox),tmp,TRUE,TRUE,0);

		SimpleComboOptions opts;
		opts.Add("180",_("180 dots per inch"));
		opts.Add("240",_("240 dots per inch"));
		opts.Add("300",_("300 dots per inch"));
		opts.Add("360",_("360 dots per inch"));
		reswidget=simplecombo_new(opts);
		g_signal_connect(G_OBJECT(reswidget),"changed",G_CALLBACK(res_changed),this);
		gtk_box_pack_start(GTK_BOX(hbox),reswidget,FALSE,FALSE,8);

		tmp=gtk_label_new(_("Click-and-drag to pan around the preview.\nRight-click to toggle magnification."));
		gtk_box_pack_start(GTK_BOX(hbox),tmp,TRUE,TRUE,24);

		tmp=gtk_button_new_with_label(_("Close"));
		g_signal_connect(G_OBJECT(tmp),"clicked",G_CALLBACK(closeclicked),this);
		gtk_box_pack_start(GTK_BOX(hbox),tmp,FALSE,FALSE,0);
//		gtk_widget_show_all(popup);

		g_signal_connect(G_OBJECT(window),"key-press-event",G_CALLBACK(keypress),this);
		g_signal_connect(G_OBJECT(window),"motion-notify-event",G_CALLBACK(mousemove),this);

		SetBG();

		drawing=close=false;

		DrawPage();
	}
	~printpreviewdata()
	{
		gtk_widget_destroy(window);
		gtk_widget_destroy(popup);
		if(factory)
			delete factory;
	}
	void SetBG()
	{
		GdkColor bgcol;
		bgcol.pixel=0;
		bgcol.red=32767;
		bgcol.green=32767;
		bgcol.blue=32767;

		CMSProfile *targetprof;
		CMColourDevice target=CM_COLOURDEVICE_NONE;
		if((targetprof=state.profilemanager.GetProfile(CM_COLOURDEVICE_PRINTERPROOF)))
			target=CM_COLOURDEVICE_PRINTERPROOF;
		else if((targetprof=state.profilemanager.GetProfile(CM_COLOURDEVICE_DISPLAY)))
			target=CM_COLOURDEVICE_DISPLAY;
		else if((targetprof=state.profilemanager.GetProfile(CM_COLOURDEVICE_DEFAULTRGB)))
			target=CM_COLOURDEVICE_DEFAULTRGB;
		if(targetprof)
			delete targetprof;

		if(target!=CM_COLOURDEVICE_NONE)
		{
			CMSTransform *transform=NULL;
			transform = factory->GetTransform(target,IS_TYPE_RGB,LCMSWRAPPER_INTENT_DEFAULT);
			if(transform)
			{
				ISDataType rgbtriple[3];
				rgbtriple[0]=bgcol.red;
				rgbtriple[1]=bgcol.green;
				rgbtriple[2]=bgcol.blue;
				transform->Transform(rgbtriple,rgbtriple,1);
				bgcol.red=rgbtriple[0];
				bgcol.green=rgbtriple[1];
				bgcol.blue=rgbtriple[2];
			}
		}
		gtk_widget_modify_bg(pview,GTK_STATE_NORMAL,&bgcol);
	}
	void DrawPage()
	{
		gtk_widget_set_sensitive(this->popup,false);
		drawing=true;

		int page=gtk_spin_button_get_value(GTK_SPIN_BUTTON(pagewidget));
		int resolutions[]={180,240,300,360};
		int idx=simplecombo_get_index(SIMPLECOMBO(reswidget));
		int res=resolutions[idx];

		printpreviewprogress prog(_("Drawing preview"),window);
		ImageSource *is=state.layout->GetImageSource(page-1,CM_COLOURDEVICE_PRINTERPROOF,factory,res,true);
		if(is)
		{
			GdkPixbuf *pb=pixbuf_from_imagesource(is,255,255,255,&prog);
			delete is;

			pixbufview_set_pixbuf(PIXBUFVIEW(pview),pb);
			g_object_unref(G_OBJECT(pb));
		}
		else
			Debug[ERROR] << "Failed to obtain imagesource for page" << page << endl;
		gtk_widget_set_sensitive(this->popup,true);
		drawing=false;
		if(close)
			delete this;
	}
	static void page_changed(GtkWidget *wid,gpointer userdata)
	{
		printpreviewdata *pv=(printpreviewdata *)userdata;
		pv->DrawPage();
	}
	static void res_changed(GtkWidget *wid,gpointer userdata)
	{
		printpreviewdata *pv=(printpreviewdata *)userdata;
		pv->DrawPage();
	}
	static void closeclicked(GtkWidget *wid,gpointer userdata)
	{
		printpreviewdata *pv=(printpreviewdata *)userdata;
		pv->close=true;
		if(!pv->drawing)
			delete pv;
	}
	static gboolean deleteevent(GtkWidget *wid,GdkEvent *ev,gpointer userdata)
	{
		printpreviewdata *pv=(printpreviewdata *)userdata;
		pv->close=true;
		if(!pv->drawing)
			delete pv;
		return(TRUE);	// Don't want the default deletion to happen as well!
	}
	static gboolean keypress(GtkWidget *widget,GdkEventKey *key, gpointer userdata)
	{
		printpreviewdata *pv=(printpreviewdata *)userdata;
		if(key->type == GDK_KEY_PRESS)
		{
			switch(key->keyval)
			{
				case GDK_Escape:
					delete pv;
				break;
			}
		}
		return FALSE;
	}
	static gboolean mousemove(GtkWidget *widget,GdkEventMotion *event, gpointer userdata)
	{
		printpreviewdata *pv=(printpreviewdata *)userdata;

		int x,y;
		GdkModifierType mods;
		gdk_window_get_pointer (widget->window, &x, &y, &mods);
		int w,h;
		gtk_window_get_size(GTK_WINDOW(pv->window),&w,&h);

		if(pv->popupshown && y>(h/8))
		{
			gtk_widget_hide_all(pv->popup);
			pv->popupshown=false;
		}

		if(!pv->popupshown && y<(h/8))
		{
			gtk_widget_show_all(pv->popup);
			pv->popupshown=true;
		}

		return(FALSE);
	}
	protected:
	PhotoPrint_State &state;
	GtkWidget *window;
	GtkWidget *popup;
	bool popupshown;
	GtkWidget *pview;
	GtkWidget *pagewidget;
	GtkWidget *reswidget;
	CMTransformFactory *factory;
	bool drawing;
	bool close;
};


void PrintPreview_Dialog(GtkWindow *parent,PhotoPrint_State &state)
{
	new printpreviewdata(state);
}

