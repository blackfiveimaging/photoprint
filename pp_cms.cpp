/*
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <string.h>

#include <gtk/gtk.h>
#include <gtk/gtkframe.h>
#include <gtk/gtksizegroup.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtkfilesel.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtklabel.h>
#include <gtk/gtktable.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtkmenuitem.h>

#include "pp_cms.h"
#include "profilemanager/profileselector.h"
#include "profilemanager/intentselector.h"
#include "dialogs.h"

#include "config.h"
#include "gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)

enum {
	CHANGED_SIGNAL,
	LAST_SIGNAL
};

static guint pp_cms_signals[LAST_SIGNAL] = { 0 };

static void pp_cms_class_init (pp_CMSClass *klass);
static void pp_cms_init (pp_CMS *stpuicombo);


static void cms_changed(GtkWidget *wid,gpointer *ob)
{
	pp_CMS *lo=(pp_CMS *)ob;

	pp_cms_refresh(lo);

	g_signal_emit(G_OBJECT (ob),pp_cms_signals[CHANGED_SIGNAL], 0);
}


void pp_cms_refresh(pp_CMS *ob)
{
	int pa=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ob->printeractive));
	int ra=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ob->rgbactive));
	int ca=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ob->cmykactive));
	int ma=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ob->monitoractive));

	gtk_widget_set_sensitive(ob->printerprof,pa);
	gtk_widget_set_sensitive(ob->intent,pa);
	gtk_widget_set_sensitive(ob->displaymode,ma);
	gtk_widget_set_sensitive(ob->colourspace,!pa);

	gtk_widget_set_sensitive(ob->rgbprof,ra);
	gtk_widget_set_sensitive(ob->cmykprof,ca);
	gtk_widget_set_sensitive(ob->monitorprof,ma);
}


GtkWidget*
pp_cms_new (ProfileManager *pm)
{
	pp_CMS *ob=PP_CMS(g_object_new (pp_cms_get_type (), NULL));
	gtk_box_set_spacing(GTK_BOX(ob),5);

	GtkSizeGroup *sizegroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	GtkWidget *frame;
	GtkWidget *label;
	GtkWidget *hbox;
	GtkWidget *vbox;

	// Printer frame
	
	frame=gtk_frame_new(_("Printer"));
	gtk_container_set_border_width(GTK_CONTAINER(frame),5);
	gtk_box_pack_start(GTK_BOX(ob),frame,FALSE,FALSE,0);
	gtk_widget_show(frame);

	vbox=gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox);
	gtk_widget_show(vbox);

	// Checkbox and file entry.

	hbox=gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,TRUE,TRUE,0);
	gtk_widget_show(hbox);

	ob->printeractive=gtk_check_button_new_with_label(_("Printer Profile:"));
	g_signal_connect(G_OBJECT(ob->printeractive),"toggled",G_CALLBACK(cms_changed),ob);
	gtk_size_group_add_widget(GTK_SIZE_GROUP(sizegroup),ob->printeractive);
	gtk_box_pack_start(GTK_BOX(hbox),ob->printeractive,FALSE,FALSE,5);
	gtk_widget_show(ob->printeractive);

	ob->printerprof=profileselector_new(pm,IS_TYPE_NULL,true);
	gtk_box_pack_start(GTK_BOX(hbox),ob->printerprof,TRUE,TRUE,5);
	gtk_widget_show(ob->printerprof);

	// Rendering Intent

	hbox=gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,TRUE,TRUE,5);
	gtk_widget_show(hbox);

	label=gtk_label_new(_("Rendering intent:"));
	gtk_size_group_add_widget(GTK_SIZE_GROUP(sizegroup),label);
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,5);
	gtk_widget_show(label);		

	ob->intent = intentselector_new(pm);
	gtk_box_pack_start(GTK_BOX(hbox),ob->intent,TRUE,TRUE,5);
	gtk_widget_show(ob->intent);

	// Printer Colour Space

	hbox=gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,TRUE,TRUE,5);
	gtk_widget_show(hbox);

	label=gtk_label_new(_("Colour space:"));
	gtk_size_group_add_widget(GTK_SIZE_GROUP(sizegroup),label);
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,5);
	gtk_widget_show(label);		

	ob->colourspace = gtk_option_menu_new ();
	GtkWidget *menu = gtk_menu_new ();

	GtkWidget *menu_item = gtk_menu_item_new_with_label ("RGB");
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
	gtk_widget_show (menu_item);
	menu_item = gtk_menu_item_new_with_label ("CMYK");
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
	gtk_widget_show (menu_item);
	
	gtk_option_menu_set_menu (GTK_OPTION_MENU (ob->colourspace), menu);
	
	gtk_box_pack_start(GTK_BOX(hbox),ob->colourspace,TRUE,TRUE,5);
	gtk_widget_show(ob->colourspace);


	// Monitor frame
	
	frame=gtk_frame_new(_("Monitor"));
	gtk_container_set_border_width(GTK_CONTAINER(frame),5);
	gtk_box_pack_start(GTK_BOX(ob),frame,FALSE,FALSE,0);
	gtk_widget_show(frame);

	vbox=gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox);
	gtk_widget_show(vbox);

	hbox=gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,TRUE,TRUE,5);
	gtk_widget_show(hbox);

	ob->monitoractive=gtk_check_button_new_with_label(_("Monitor Profile:"));
	g_signal_connect(G_OBJECT(ob->monitoractive),"toggled",G_CALLBACK(cms_changed),ob);
	gtk_size_group_add_widget(GTK_SIZE_GROUP(sizegroup),ob->monitoractive);
	gtk_box_pack_start(GTK_BOX(hbox),ob->monitoractive,FALSE,FALSE,5);
	gtk_widget_show(ob->monitoractive);

	ob->monitorprof=profileselector_new(pm,IS_TYPE_RGB,false);
	gtk_box_pack_start(GTK_BOX(hbox),ob->monitorprof,TRUE,TRUE,5);
	gtk_widget_show(ob->monitorprof);

	hbox=gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,TRUE,TRUE,5);
	gtk_widget_show(hbox);

	label=gtk_label_new(_("Display mode:"));
	gtk_size_group_add_widget(GTK_SIZE_GROUP(sizegroup),label);
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,5);
	gtk_widget_show(label);

	ob->displaymode = gtk_option_menu_new ();
	menu = gtk_menu_new ();

	menu_item = gtk_menu_item_new_with_label (_("Normal"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
	gtk_widget_show (menu_item);
	menu_item = gtk_menu_item_new_with_label (_("Simulate Print"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
	gtk_widget_show (menu_item);
	menu_item = gtk_menu_item_new_with_label (_("Simulate Print, Adapt White"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
	gtk_widget_show (menu_item);

	gtk_option_menu_set_menu (GTK_OPTION_MENU (ob->displaymode), menu);
	
	gtk_box_pack_start(GTK_BOX(hbox),ob->displaymode,TRUE,TRUE,5);
	gtk_widget_show(ob->displaymode);


	// Default Profiles frame
	
	frame=gtk_frame_new(_("Default Image Profiles"));
	gtk_container_set_border_width(GTK_CONTAINER(frame),5);
	gtk_box_pack_start(GTK_BOX(ob),frame,FALSE,FALSE,0);
	gtk_widget_show(frame);

	vbox=gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame),vbox);
	gtk_widget_show(vbox);

	// RGB Profile - Checkbox and file entry.

	hbox=gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,TRUE,TRUE,0);
	gtk_widget_show(hbox);

	ob->rgbactive=gtk_check_button_new_with_label(_("RGB Profile:"));
	g_signal_connect(G_OBJECT(ob->rgbactive),"toggled",G_CALLBACK(cms_changed),ob);
	gtk_size_group_add_widget(GTK_SIZE_GROUP(sizegroup),ob->rgbactive);
	gtk_box_pack_start(GTK_BOX(hbox),ob->rgbactive,FALSE,FALSE,5);
	gtk_widget_show(ob->rgbactive);

	ob->rgbprof=profileselector_new(pm,IS_TYPE_RGB,false);
	gtk_box_pack_start(GTK_BOX(hbox),ob->rgbprof,TRUE,TRUE,5);
	gtk_widget_show(ob->rgbprof);

	// CMYK Profile - Checkbox and file entry.

	hbox=gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,TRUE,TRUE,5);
	gtk_widget_show(hbox);

	ob->cmykactive=gtk_check_button_new_with_label(_("CMYK Profile:"));
	g_signal_connect(G_OBJECT(ob->cmykactive),"toggled",G_CALLBACK(cms_changed),ob);
	gtk_size_group_add_widget(GTK_SIZE_GROUP(sizegroup),ob->cmykactive);
	gtk_box_pack_start(GTK_BOX(hbox),ob->cmykactive,FALSE,FALSE,5);
	gtk_widget_show(ob->cmykactive);

	ob->cmykprof=profileselector_new(pm,IS_TYPE_CMYK,false);
	gtk_box_pack_start(GTK_BOX(hbox),ob->cmykprof,TRUE,TRUE,5);
	gtk_widget_show(ob->cmykprof);


	g_object_unref(G_OBJECT(sizegroup));

	pp_cms_refresh(ob);

	return(GTK_WIDGET(ob));
}


GType
pp_cms_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo pp_cms_info =
		{
			sizeof (pp_CMSClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) pp_cms_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (pp_CMS),
			0,
			(GInstanceInitFunc) pp_cms_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_VBOX, "pp_CMS", &pp_cms_info, (GTypeFlags)0);
	}
	return stpuic_type;
}


static void
pp_cms_class_init (pp_CMSClass *klass)
{
	pp_cms_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (klass),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (pp_CMSClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
pp_cms_init (pp_CMS *ob)
{
}


void pp_cms_populate(pp_CMS *ob,PhotoPrint_State *db)
{
	const char *pf;
	char *pf2;

	cerr << "Populating PP_CMS..." << endl;

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ob->printeractive),db->profilemanager.FindInt("PrinterProfileActive"));
	pf=db->profilemanager.FindString("PrinterProfile");
	if(pf && strlen(pf))
	{
		if((pf2=db->profilemanager.Search(pf)))
		{
			cerr << "Setting printer profile to " << pf2 << endl;
			profileselector_set_filename(PROFILESELECTOR(ob->printerprof),pf2);
			free(pf2);
		}
		else
			profileselector_set_filename(PROFILESELECTOR(ob->printerprof),pf);
	}

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ob->rgbactive),db->profilemanager.FindInt("DefaultRGBProfileActive"));
	pf=db->profilemanager.FindString("DefaultRGBProfile");
	cerr << "Default RGB Profile" << pf;
	if(pf && strlen(pf))
	{
		if((pf2=db->profilemanager.Search(pf)))
		{
			cerr << "Setting RGB profile to " << pf2 << endl;
			profileselector_set_filename(PROFILESELECTOR(ob->rgbprof),pf2);
			free(pf2);
		}
		else
			profileselector_set_filename(PROFILESELECTOR(ob->rgbprof),pf);
	}

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ob->cmykactive),db->profilemanager.FindInt("DefaultCMYKProfileActive"));
	pf=db->profilemanager.FindString("DefaultCMYKProfile");
	cerr << "Default CMYK Profile" << pf;
	if(pf && strlen(pf))
	{
		if((pf2=db->profilemanager.Search(pf)))
		{
			cerr << "Setting CMYK profile to " << pf2 << endl;
			profileselector_set_filename(PROFILESELECTOR(ob->cmykprof),pf2);
			free(pf2);
		}
		else
			profileselector_set_filename(PROFILESELECTOR(ob->cmykprof),pf);
	}

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ob->monitoractive),db->profilemanager.FindInt("MonitorProfileActive"));
	pf=db->profilemanager.FindString("MonitorProfile");
	if(pf && strlen(pf))
	{
		if((pf2=db->profilemanager.Search(pf)))
		{
			cerr << "Setting Monitor profile to " << pf2 << endl;
			profileselector_set_filename(PROFILESELECTOR(ob->monitorprof),pf2);
			free(pf2);
		}
		else
			profileselector_set_filename(PROFILESELECTOR(ob->monitorprof),pf);
	}

	intentselector_setintent(INTENTSELECTOR(ob->intent),LCMSWrapper_Intent(db->profilemanager.FindInt("RenderingIntent")));

	gtk_option_menu_set_history(GTK_OPTION_MENU(ob->displaymode),db->profilemanager.FindInt("ProofMode"));

	const char *cs=db->FindString("PrintColourSpace");
	if(strcmp(cs,"RGB")==0)
		gtk_option_menu_set_history(GTK_OPTION_MENU(ob->colourspace),0);
	else if(strcmp(cs,"CMYK")==0)
		gtk_option_menu_set_history(GTK_OPTION_MENU(ob->colourspace),1);
	else
		cerr << "Warning: invalid colour space" << endl;
}


void pp_cms_depopulate(pp_CMS *ob,PhotoPrint_State *db)
{
	char *pf=NULL;

	db->profilemanager.SetInt("PrinterProfileActive",gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ob->printeractive)));
	pf=db->profilemanager.MakeRelative(profileselector_get_filename(PROFILESELECTOR(ob->printerprof)));
	if(pf)
		cerr << "Printer profile: " <<  pf << endl;
	db->profilemanager.SetString("PrinterProfile",pf);
	free(pf);

	db->profilemanager.SetInt("DefaultRGBProfileActive",gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ob->rgbactive)));
	pf=db->profilemanager.MakeRelative(profileselector_get_filename(PROFILESELECTOR(ob->rgbprof)));
	if(pf)
		cerr << "RGB profile: " <<  pf << endl;
	db->profilemanager.SetString("DefaultRGBProfile",pf);
	free(pf);

	db->profilemanager.SetInt("DefaultCMYKProfileActive",gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ob->cmykactive)));
	pf=db->profilemanager.MakeRelative(profileselector_get_filename(PROFILESELECTOR(ob->cmykprof)));
	if(pf)
		cerr << "CMYK profile: " <<  pf << endl;
	db->profilemanager.SetString("DefaultCMYKProfile",pf);
	free(pf);

	db->profilemanager.SetInt("MonitorProfileActive",gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ob->monitoractive)));
	pf=db->profilemanager.MakeRelative(profileselector_get_filename(PROFILESELECTOR(ob->monitorprof)));
	if(pf)
		cerr << "Monitor profile: " <<  pf << endl;
	db->profilemanager.SetString("MonitorProfile",pf);
	free(pf);

	db->profilemanager.SetInt("RenderingIntent",intentselector_getintent(INTENTSELECTOR(ob->intent)));

	db->profilemanager.SetInt("ProofMode",gtk_option_menu_get_history(GTK_OPTION_MENU(ob->displaymode)));

	int cs=gtk_option_menu_get_history(GTK_OPTION_MENU(ob->colourspace));
	switch(cs)
	{
		case 0:
			db->SetString("PrintColourSpace","RGB");
			break;
		case 1:
			db->SetString("PrintColourSpace","CMYK");
			break;
	}
}


void pp_cms_run_dialog(PhotoPrint_State *db,GtkWindow *parent)
{
	GtkWidget *dialog=gtk_dialog_new_with_buttons(_("Colour Management"),
		parent,GtkDialogFlags(0),
		GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
		GTK_STOCK_OK,GTK_RESPONSE_OK,
		NULL);

	GtkWidget *cms=pp_cms_new(&db->profilemanager);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),cms,FALSE,FALSE,0);
	gtk_widget_show(cms);

	pp_cms_populate(PP_CMS(cms),db);

	gtk_widget_show(dialog);
	gint result=gtk_dialog_run(GTK_DIALOG(dialog));
	switch(result)
	{
		case GTK_RESPONSE_CANCEL:
			break;
		case GTK_RESPONSE_OK:
			pp_cms_depopulate(PP_CMS(cms),db);
			break;
	}
	gtk_widget_destroy(dialog);
}