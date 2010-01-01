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
#include "miscwidgets/simplecombo.h"
#include "profilemanager/profileselector.h"
#include "profilemanager/intentselector.h"
#include "dialogs.h"

#include "support/debug.h"

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
	Debug[TRACE] << "In pp_cms_refresh" << endl;

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

	IS_TYPE colourspace=simplecombo_get_index(SIMPLECOMBO(ob->colourspace)) ? IS_TYPE_CMYK : IS_TYPE_RGB;
	IS_TYPE dlcolourspace=colourspace;
	bool isdevicelink=false;

	Debug[TRACE] << "Getting profiles from widgets..." << endl;

	// Get colourspace from printer profile, if set.
	if(pa)
	{
		Debug[TRACE] << "Getting printer profile..." << endl;
		CMSProfile *p=ob->pm->GetProfile(profileselector_get_filename(PROFILESELECTOR(ob->printerprof)));
		if(p)
		{
			Debug[TRACE] << "Got printer profile..." << endl;
			colourspace=p->GetColourSpace();
			if((isdevicelink=p->IsDeviceLink()))
			{
				dlcolourspace=colourspace;
				colourspace=p->GetDeviceLinkOutputSpace();
			}
			delete p;
		}
		else
		{
			Debug[TRACE] << "Couldn't get printer profile..." << endl;
			pa=false;
		}
	}

	// Check RGB profile...
	if(ra)
	{
		CMSProfile *p=ob->pm->GetProfile(profileselector_get_filename(PROFILESELECTOR(ob->rgbprof)));
		if(p)
			delete p;
		else
			ra=false;
	}

	// Check CMYK profile...
	if(ra)
	{
		CMSProfile *p=ob->pm->GetProfile(profileselector_get_filename(PROFILESELECTOR(ob->cmykprof)));
		if(p)
			delete p;
		else
			ca=false;
	}

	// Check Monitor profile...
	if(ra)
	{
		CMSProfile *p=ob->pm->GetProfile(profileselector_get_filename(PROFILESELECTOR(ob->monitorprof)));
		if(p)
			delete p;
		else
			ma=false;
	}

	Debug[TRACE] << "Got colourspace: " << colourspace << endl;

	const gchar *rgbok=GTK_STOCK_NO;
	const char *rgbstatus="";

	// Work out implications of current settings for RGB images
	if(isdevicelink)	// implies printer active...
	{
		if(dlcolourspace==IS_TYPE_CMYK)
		{
			if(ca && ra)	// do we have both RGB and CMYK default profiles
			{
				// Devicelink with CMYK input, CMYK profile available
				rgbok=GTK_STOCK_DIALOG_WARNING;
				rgbstatus=_("Printer profile is a devicelink with CMYK input - RGB images\nwill first be converted to the default CMYK profile");
			}
			else if(ra)
			{
				// Devicelink with CMYK input, no CMYK profile available
				rgbok=GTK_STOCK_NO;
				rgbstatus=_("Printer profile is a devicelink with CMYK input - RGB images\ncannot be printed without a default CMYK profile.");
			}
			else
			{
				// Devicelink with CMYK input, no RGB or CMYK profile available
				rgbok=GTK_STOCK_NO;
				rgbstatus=_("Printer profile is a devicelink with CMYK input - RGB images\ncannot be printed without a default RGB and CMYK profile.");
				// Exception: input RGB files with embedded profiles - but explaining that would be complicated.
			}
		}
		else if(ra)
		{
			// Devicelink with RGB input
			rgbok=GTK_STOCK_DIALOG_WARNING;
			rgbstatus=_("Printer profile is a devicelink - RGB images with embedded profiles\nwill first be converted to the default RGB profile.");
		}
		else
		{
			// Devicelink with RGB input
			rgbok=GTK_STOCK_DIALOG_WARNING;
			rgbstatus=_("Printer profile is a devicelink - RGB images with embedded profiles will not\nprint correctly, since there is no default RGB profile.");
		}
	}
	else if(pa && ra)
	{
		rgbok=GTK_STOCK_YES;
		rgbstatus=_("RGB images will print correctly");
	}
	else if(pa && colourspace==IS_TYPE_RGB)
	{
		rgbok=GTK_STOCK_DIALOG_WARNING;
		rgbstatus=_("RGB images can be printed but colours depend on the driver\n(Colours will be accurate for images that contain an embedded profile)");
	}
	else if(colourspace==IS_TYPE_RGB)
	{
		rgbok=GTK_STOCK_DIALOG_WARNING;
		rgbstatus=_("RGB images can be printed but colours depend on the driver");
	}
	else if(pa && colourspace==IS_TYPE_CMYK)
	{
		rgbok=GTK_STOCK_DIALOG_WARNING;
		rgbstatus=_("RGB images can only be printed if they have an embedded profile\n(Workflow is CMYK and there is no default RGB profile)");
	}
	else if(colourspace==IS_TYPE_CMYK)
	{
		rgbok=GTK_STOCK_NO;
		rgbstatus=_("RGB images cannot be printed\n(Workflow is CMYK and there is no printer profile)");
	}

	const gchar *cmykok=GTK_STOCK_NO;
	const char *cmykstatus="";

	// Work out implications of current settings for CMYK images...
	if(isdevicelink)
	{
		if(dlcolourspace==IS_TYPE_RGB)
		{
			if(ra && ca)
			{
				// Devicelink with RGB input, CMYK profile available
				cmykok=GTK_STOCK_DIALOG_WARNING;
				cmykstatus=_("Printer profile is a devicelink with RGB input - CMYK images\nwill first be converted to the default RGB profile");
			}
			else if(ca)
			{
				// Devicelink with RGB input, no CMYK profile available
				cmykok=GTK_STOCK_NO;
				cmykstatus=_("Printer profile is a devicelink with RGB input - CMYK images\ncannot be printed without a default RGB profile.");
			}
			else
			{
				// Devicelink with RGB input, no RGB or CMYK profile available
				cmykok=GTK_STOCK_NO;
				cmykstatus=_("Printer profile is a devicelink with RGB input - CMYK images\ncannot be printed without a default RGB and CMYK profile.");
			}
		}
		else if(ca)
		{
			// Devicelink with CMYK input
			cmykok=GTK_STOCK_DIALOG_WARNING;
			cmykstatus=_("Printer profile is a devicelink - CMYK images with embedded profiles\nwill first be converted to the default CMYK profile.");
		}
		else
		{
			// Devicelink with CMYK input
			cmykok=GTK_STOCK_DIALOG_WARNING;
			cmykstatus=_("Printer profile is a devicelink - CMYK images with embedded profiles will not\nprint correctly since there is no default CMYK profile.");
		}
	}
	else if(pa && ca)
	{
		cmykok=GTK_STOCK_YES;
		cmykstatus=_("CMYK images will print correctly");
	}
	else if(pa && colourspace==IS_TYPE_CMYK)
	{
		cmykok=GTK_STOCK_DIALOG_WARNING;
		cmykstatus=_("CMYK images can be printed but colours depend on the driver\n(Colours will be accurate for images that contain an embedded profile)");
	}
	else if(colourspace==IS_TYPE_CMYK)
	{
		cmykok=GTK_STOCK_DIALOG_WARNING;
		cmykstatus=_("CMYK images can be printed but colours depend on the driver");
	}
	else if(pa && colourspace==IS_TYPE_RGB)
	{
		cmykok=GTK_STOCK_DIALOG_WARNING;
		cmykstatus=_("CMYK images can only be printed if they have an embedded profile\n(Workflow is RGB and there is no default CMYK profile)");
	}
	else if(colourspace==IS_TYPE_RGB)
	{
		cmykok=GTK_STOCK_NO;
		cmykstatus=_("CMYK images cannot be printed\n(Workflow is RGB and there is no printer profile)");
	}


	const gchar *monok=GTK_STOCK_NO;
	const char *monstatus="";

	if(ma && ra)
	{
		monok=GTK_STOCK_YES;
		monstatus=_("Images will be displayed correctly");
	}
	else if(ma)
	{
		monok=GTK_STOCK_DIALOG_WARNING;
		monstatus=_("Images will only be displayed correctly if they have an embedded profile");
	}
	else
	{
		monok=GTK_STOCK_NO;
		monstatus=_("Images will not be displayed correctly");
	}


	gtk_label_set_text(GTK_LABEL(ob->statusline[0]),rgbstatus);
	gtk_image_set_from_stock(GTK_IMAGE(ob->indicator[0]),rgbok, GTK_ICON_SIZE_SMALL_TOOLBAR);

	gtk_label_set_text(GTK_LABEL(ob->statusline[1]),cmykstatus);
	gtk_image_set_from_stock(GTK_IMAGE(ob->indicator[1]),cmykok, GTK_ICON_SIZE_SMALL_TOOLBAR);

	gtk_label_set_text(GTK_LABEL(ob->statusline[2]),monstatus);
	gtk_image_set_from_stock(GTK_IMAGE(ob->indicator[2]),monok, GTK_ICON_SIZE_SMALL_TOOLBAR);
}


GtkWidget*
pp_cms_new (ProfileManager *pm)
{
	pp_CMS *ob=PP_CMS(g_object_new (pp_cms_get_type (), NULL));
	gtk_box_set_spacing(GTK_BOX(ob),5);

	ob->pm=pm;

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
	g_signal_connect(G_OBJECT(ob->printerprof),"changed",G_CALLBACK(cms_changed),ob);
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

	// Colourspace

	SimpleComboOptions csopts;
	csopts.Add("RGB",_("RGB"),_("Send Red, Green and Blue data to the printer driver"));
	csopts.Add("CMYK",_("CMYK"),_("Send Cyan, Magenta, Yellow and Black data to the printer driver"));

	ob->colourspace=simplecombo_new(csopts);
	g_signal_connect(G_OBJECT(ob->colourspace),"changed",G_CALLBACK(cms_changed),ob);
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
	g_signal_connect(G_OBJECT(ob->monitorprof),"changed",G_CALLBACK(cms_changed),ob);
	gtk_box_pack_start(GTK_BOX(hbox),ob->monitorprof,TRUE,TRUE,5);
	gtk_widget_show(ob->monitorprof);

	hbox=gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,TRUE,TRUE,5);
	gtk_widget_show(hbox);

	label=gtk_label_new(_("Display mode:"));
	gtk_size_group_add_widget(GTK_SIZE_GROUP(sizegroup),label);
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,5);
	gtk_widget_show(label);


	// Proof mode

	SimpleComboOptions pmopts;
	pmopts.Add("Normal",_("Normal"),_("Displays images' original colours, transformed to suit the monitor"));
	pmopts.Add("SimulatePrint",_("Simulate Print"),_("Simulates how colours will look when printed, imitating the paper's white point."));
	pmopts.Add("SimulatePrintAdaptWhite",_("Simulate Print, Adapt White"),_("Simulates how colours will look when printed, adapting colours for the monitor's white point."));

	ob->displaymode=simplecombo_new(pmopts);

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
	g_signal_connect(G_OBJECT(ob->rgbprof),"changed",G_CALLBACK(cms_changed),ob);
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
	g_signal_connect(G_OBJECT(ob->cmykprof),"changed",G_CALLBACK(cms_changed),ob);
	gtk_box_pack_start(GTK_BOX(hbox),ob->cmykprof,TRUE,TRUE,5);
	gtk_widget_show(ob->cmykprof);


	GtkWidget *table=gtk_table_new(3,3,FALSE);
	gtk_table_set_col_spacings(GTK_TABLE(table),12);
	gtk_table_set_row_spacings(GTK_TABLE(table),3);
	gtk_box_pack_start(GTK_BOX(ob),table,FALSE,FALSE,3);
	gtk_widget_show(table);

	for(int i=0;i<3;++i)
	{
		GtkAttachOptions gao = (GtkAttachOptions)(GTK_EXPAND|GTK_FILL);

		ob->indicator[i]=gtk_image_new();
		gtk_table_attach(GTK_TABLE(table),ob->indicator[i],1,2,i,i+1,GTK_SHRINK,gao,0,0);
		gtk_widget_show(ob->indicator[i]);

		ob->statusline[i]=gtk_label_new("");
		gtk_misc_set_alignment(GTK_MISC(ob->statusline[i]),0,0.5);
		gtk_table_attach(GTK_TABLE(table),ob->statusline[i],2,3,i,i+1,gao,gao,0,0);
		gtk_widget_show(ob->statusline[i]);
	}

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

	Debug[TRACE] << "Populating PP_CMS..." << endl;

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ob->printeractive),db->profilemanager.FindInt("PrinterProfileActive"));
	pf=db->profilemanager.FindString("PrinterProfile");
	if(pf && strlen(pf))
	{
		if((pf2=db->profilemanager.SearchPaths(pf)))
		{
			Debug[TRACE] << "Setting printer profile to " << pf2 << endl;
			profileselector_set_filename(PROFILESELECTOR(ob->printerprof),pf2);
			free(pf2);
		}
		else
			profileselector_set_filename(PROFILESELECTOR(ob->printerprof),pf);
	}

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ob->rgbactive),db->profilemanager.FindInt("DefaultRGBProfileActive"));
	pf=db->profilemanager.FindString("DefaultRGBProfile");
	Debug[TRACE] << "Default RGB Profile" << pf;
	if(pf && strlen(pf))
	{
		if((pf2=db->profilemanager.SearchPaths(pf)))
		{
			Debug[TRACE] << "Setting RGB profile to " << pf2 << endl;
			profileselector_set_filename(PROFILESELECTOR(ob->rgbprof),pf2);
			free(pf2);
		}
		else
			profileselector_set_filename(PROFILESELECTOR(ob->rgbprof),pf);
	}

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ob->cmykactive),db->profilemanager.FindInt("DefaultCMYKProfileActive"));
	pf=db->profilemanager.FindString("DefaultCMYKProfile");
	Debug[TRACE] << "Default CMYK Profile" << pf;
	if(pf && strlen(pf))
	{
		if((pf2=db->profilemanager.SearchPaths(pf)))
		{
			Debug[TRACE] << "Setting CMYK profile to " << pf2 << endl;
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
		if((pf2=db->profilemanager.SearchPaths(pf)))
		{
			Debug[TRACE] << "Setting Monitor profile to " << pf2 << endl;
			profileselector_set_filename(PROFILESELECTOR(ob->monitorprof),pf2);
			free(pf2);
		}
		else
			profileselector_set_filename(PROFILESELECTOR(ob->monitorprof),pf);
	}

	intentselector_setintent(INTENTSELECTOR(ob->intent),LCMSWrapper_Intent(db->profilemanager.FindInt("RenderingIntent")));

	simplecombo_set_index(SIMPLECOMBO(ob->displaymode),db->profilemanager.FindInt("ProofMode"));

	const char *cs=db->FindString("PrintColourSpace");
	simplecombo_set(SIMPLECOMBO(ob->colourspace),cs);
}


void pp_cms_depopulate(pp_CMS *ob,PhotoPrint_State *db)
{
	char *pf=NULL;

	db->profilemanager.SetInt("PrinterProfileActive",gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ob->printeractive)));
	pf=db->profilemanager.MakeRelative(profileselector_get_filename(PROFILESELECTOR(ob->printerprof)));
	if(pf)
		Debug[TRACE] << "Printer profile: " <<  pf << endl;
	db->profilemanager.SetString("PrinterProfile",pf);
	free(pf);

	db->profilemanager.SetInt("DefaultRGBProfileActive",gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ob->rgbactive)));
	pf=db->profilemanager.MakeRelative(profileselector_get_filename(PROFILESELECTOR(ob->rgbprof)));
	if(pf)
		Debug[TRACE] << "RGB profile: " <<  pf << endl;
	db->profilemanager.SetString("DefaultRGBProfile",pf);
	free(pf);

	db->profilemanager.SetInt("DefaultCMYKProfileActive",gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ob->cmykactive)));
	pf=db->profilemanager.MakeRelative(profileselector_get_filename(PROFILESELECTOR(ob->cmykprof)));
	if(pf)
		Debug[TRACE] << "CMYK profile: " <<  pf << endl;
	db->profilemanager.SetString("DefaultCMYKProfile",pf);
	free(pf);

	db->profilemanager.SetInt("MonitorProfileActive",gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ob->monitoractive)));
	pf=db->profilemanager.MakeRelative(profileselector_get_filename(PROFILESELECTOR(ob->monitorprof)));
	if(pf)
		Debug[TRACE] << "Monitor profile: " <<  pf << endl;
	db->profilemanager.SetString("MonitorProfile",pf);
	free(pf);

	db->profilemanager.SetInt("RenderingIntent",intentselector_getintent(INTENTSELECTOR(ob->intent)));

	// FIXME - need a better way of handling integer keys
	db->profilemanager.SetInt("ProofMode",simplecombo_get_index(SIMPLECOMBO(ob->displaymode)));

	const char *cs=simplecombo_get(SIMPLECOMBO(ob->colourspace));
	db->SetString("PrintColourSpace",cs);
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
