#include <iostream>

#include <gtk/gtk.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)

#include "imagesource_gdkpixbuf.h"
#include "pixbuf_from_imagesource.h"

#include "ppeffect_desaturate.h"

using namespace std;

struct dialogstate
{
	PPEffect_Desaturate *effect;
	GtkWidget *preview;
	GdkPixbuf *previewsourcepb;
};


static void refreshdialog(GtkWidget *wid,gpointer obj)
{
	dialogstate *st=(dialogstate *)obj;

	ImageSource *is=new ImageSource_GdkPixbuf(st->previewsourcepb);
	is=st->effect->Apply(is);
	GdkPixbuf *tn=pixbuf_from_imagesource(is);
	delete is;
	
	gtk_image_set_from_pixbuf(GTK_IMAGE(st->preview),tn);
	g_object_unref(G_OBJECT(tn));
}


GtkWidget *PPEffect_Desaturate::SettingsWidget()
{
	return(NULL);
}


bool PPEffect_Desaturate::Dialog(GtkWindow *parent,GdkPixbuf *preview)
{
#if 0
	dialogstate st;
	st.effect=this;

	GtkWidget *dialog;
//	GtkWidget *hbox;
	GtkWidget *label;
 	dialog=gtk_dialog_new_with_buttons(_("Change Colour Temperature..."),
		parent,GtkDialogFlags(0),
		GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
		GTK_STOCK_OK,GTK_RESPONSE_OK,
		NULL);

	GtkWidget *table=gtk_table_new(2,2,FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),table,FALSE,FALSE,5);
	gtk_widget_show(table);

	label=gtk_label_new(_("Before:"));
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,0,1);
	gtk_widget_show(label);

	label=gtk_label_new(_("After:"));
	gtk_table_attach_defaults(GTK_TABLE(table),label,1,2,0,1);
	gtk_widget_show(label);

	st.previewsourcepb=preview;

	GtkWidget *image1 = gtk_image_new_from_pixbuf(preview);
	GtkWidget *image2 = gtk_image_new_from_pixbuf(preview);

	gtk_table_attach_defaults(GTK_TABLE(table),image1,0,1,1,2);
	gtk_widget_show(image1);

	gtk_table_attach_defaults(GTK_TABLE(table),image2,1,2,1,2);
	gtk_widget_show(image2);
	st.preview=image2;

	st.slider=gtk_hscale_new_with_range(-1500,1500,250);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),st.slider,FALSE,FALSE,5);
	gtk_range_set_value(GTK_RANGE(st.slider),tempchange);
	g_signal_connect(GTK_WIDGET(st.slider),"value-changed",G_CALLBACK(refreshdialog),&st);

	gtk_widget_show(st.slider);

	gtk_widget_show(dialog);
	
	bool done=false;
	bool ret=false;
	while(!done)
	{
		gint result=gtk_dialog_run(GTK_DIALOG(dialog));
		switch(result)
		{
			case GTK_RESPONSE_CANCEL:
				cerr << "Clicked Cancel" << endl;
				done=true;
				ret=false;
				break;
			case GTK_RESPONSE_OK:
				cerr << "Clicked OK" << endl;
				done=true;
				ret=true;
				break;
		}
	}
	gtk_widget_destroy(dialog);

#endif
	return(true);
}

