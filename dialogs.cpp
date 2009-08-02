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

#include <sys/stat.h>

#include <gutenprint/vars.h>

#include <gtk/gtk.h>
#include <gtk/gtknotebook.h>
#include <gdk/gdkkeysyms.h>

#include "gp_cppsupport/printoutputselector.h"
#include "pp_cms.h"
#include "pp_units.h"
#include "pp_scaling.h"
#include "stpui_widgets/stpui_optionbook.h"
#include "splashscreen/splashscreen.h"

#include "miscwidgets/patheditor.h"
#include "miscwidgets/imageselector.h"
#include "miscwidgets/generaldialogs.h"
#include "miscwidgets/pixbufview.h"
#include "miscwidgets/simplecombo.h"

#include "support/pathsupport.h"
#include "support/rangeparser.h"
#include "support/progressbar.h"
#include "support/util.h"

#include "imageutils/tiffsave.h"
#include "imageutils/jpegsave.h"
#include "imagesource/pixbuf_from_imagesource.h"

#include "profilemanager/profileselector.h"
#include "profilemanager/intentselector.h"

#include "config.h"
#include "gettext.h"
#define _(x) gettext(x)

#include "dialogs.h"

using namespace std;


// Colour Management

struct colourmanagementdata
{
	PhotoPrint_State *state;
	GtkWidget *dialog;
};

void ColourManagement_Dialog(GtkWindow *parent,PhotoPrint_State &state)
{
	pp_cms_run_dialog(&state,parent);
}

void Units_Dialog(GtkWindow *parent,PhotoPrint_State &state)
{
	enum Units units=state.GetUnits();
	enum Units newunits=pp_units_run_dialog(units,parent);
	if(units!=newunits)
		state.SetUnits(newunits);
}


void Scaling_Dialog(GtkWindow *parent,PhotoPrint_State &state)
{
	IS_ScalingQuality q=IS_ScalingQuality(state.FindInt("ScalingQuality"));
	q=pp_scaling_run_dialog(parent,q);
	state.SetInt("ScalingQuality",q);
}

//  Print Setup


struct printsetupdata
{
	PhotoPrint_State *state;
	GtkWidget *optionbook;
	GtkWidget *printoutput;
	GtkWidget *dialog;
	stp_vars_t *backupvars;
	char *olddriver;
	stpui_optionbook_custompage custompage;
};


static void driver_changed(GtkWidget *wid,gpointer data)
{
	cerr << "In driver_changed()" << endl;

	struct printsetupdata *cbd=(struct printsetupdata *)data;
	PrintOutput *po=&cbd->state->printoutput;

	const char *driver=po->FindString("Driver");
	cerr << "Driver=" << driver << endl;

	if(!(cbd->state->printer.SetDriver(po->FindString("Driver"))))
	{
		cerr << "Setting driver failed - reverting to default" << endl;
		po->SetString("Driver",DEFAULT_PRINTER_DRIVER);
		cbd->state->printer.SetDriver(po->FindString("Driver"));
		printoutputselector_refresh(PRINTOUTPUTSELECTOR(cbd->printoutput));
	}

	stpui_optionbook_rebuild(STPUI_OPTIONBOOK(cbd->optionbook));	
}


void PrintSetup_Dialog(GtkWindow *parent,PhotoPrint_State &state)
{
	printsetupdata dialogdata;

	cerr << "Opening print setup dialog" << endl;
	
	dialogdata.state=&state;
	dialogdata.backupvars=stp_vars_create_copy(state.printer.stpvars);
	dialogdata.olddriver=g_strdup(state.printoutput.FindString("Driver"));
	
	dialogdata.dialog=gtk_dialog_new_with_buttons(_("Printer Setup"),
		parent,GtkDialogFlags(0),
		GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
		GTK_STOCK_OK,GTK_RESPONSE_OK,
		NULL);
	gtk_window_set_default_size(GTK_WINDOW(dialogdata.dialog),500,350);

	cerr << "Creating PrintOutput widget..." << endl;

	dialogdata.printoutput=printoutputselector_new(&state.printoutput);
	g_object_ref(G_OBJECT(dialogdata.printoutput));

	cerr << "Created PrintOutput widget..." << endl;

	dialogdata.custompage.name=_("Output");
	dialogdata.custompage.widget=dialogdata.printoutput;
	
	dialogdata.optionbook=stpui_optionbook_new(state.printer.stpvars,&dialogdata.custompage,1);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialogdata.dialog)->vbox),dialogdata.optionbook,TRUE,TRUE,0);
	gtk_widget_show(dialogdata.optionbook);
	
	g_signal_connect(G_OBJECT(dialogdata.printoutput),"changed",G_CALLBACK(driver_changed),&dialogdata);

	gtk_widget_show(dialogdata.dialog);
	gint result=gtk_dialog_run(GTK_DIALOG(dialogdata.dialog));
	switch(result)
	{
		case GTK_RESPONSE_CANCEL:
			state.printoutput.DBToQueues();
			stp_vars_copy(state.printer.stpvars,dialogdata.backupvars);
			state.printoutput.SetString("Driver",dialogdata.olddriver);
			break;
		case GTK_RESPONSE_OK:
			state.printoutput.QueuesToDB();
			const char *p1=stp_get_string_parameter(dialogdata.backupvars,"PageSize");
			const char *p2=stp_get_string_parameter(state.printer.stpvars,"PageSize");
			if(p1 && p2 && strcmp(p1,p2)!=0)
				state.layout->UpdatePageSize();  // Needs to be run every time, but preserve margins.
			break;
	}
	g_object_unref(G_OBJECT(dialogdata.printoutput));
	stp_vars_destroy(dialogdata.backupvars);
	g_free(dialogdata.olddriver);
	gtk_widget_destroy(dialogdata.dialog);
}


// About dialog

void About_Dialog(GtkWindow *parent)
{
	GtkWidget *dialog=gtk_dialog_new_with_buttons(_("About"),
		parent,GtkDialogFlags(0),
		GTK_STOCK_OK,GTK_RESPONSE_OK,
		NULL);

	GdkPixbuf *pb=SplashScreen::GetPixbuf();

	GtkWidget *image=gtk_image_new_from_pixbuf(pb);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),image,FALSE,FALSE,0);
	gtk_widget_show(image);
	
	GtkWidget *label=gtk_label_new(_("PhotoPrint "
	PACKAGE_VERSION
	" - copyright (c) 2004-2009\n"
	"by Alastair M. Robinson (amr@blackfiveservices.co.uk)\nDistributed under the terms\nof the GNU General Public Licence.\n"
	"See the file 'COPYING' for more details.\n"
	"The borders pack is released under the Creative Commons\nAttribution 2.0 UK: England & Wales Licence"));
	gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_CENTER);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),label,TRUE,FALSE,5);
	gtk_widget_show(label);

	gtk_widget_show(dialog);
	gtk_dialog_run(GTK_DIALOG(dialog));

	gtk_widget_destroy(dialog);
	g_object_unref(G_OBJECT(pb));
}

// FIXME!  Horrible hack to get around a symbol clash on Win32.
#define SearchPathHandlerA SearchPathHandler

struct PathDialogData
{
	SearchPathHandler sp;
	ImageSelector *is;
	PathEditor *pe;
};


static void	paths_changed(GtkWidget *widget,gpointer user_data)
{
	PathDialogData *dd=(PathDialogData *)user_data;
	patheditor_get_paths(dd->pe,&dd->sp);
	imageselector_refresh(IMAGESELECTOR(dd->is));
}


void Paths_Dialog(GtkWindow *parent,PhotoPrint_State &state)
{
	GtkWidget *dialog;
	dialog=gtk_dialog_new_with_buttons(_("Set paths..."),
		parent,GtkDialogFlags(0),
		GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
		GTK_STOCK_OK,GTK_RESPONSE_OK,
		NULL);
	gtk_window_set_default_size(GTK_WINDOW(dialog),500,350);

	GtkWidget *book=gtk_notebook_new();
	
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),book,TRUE,TRUE,0);
	gtk_widget_show(book);

	GtkWidget *label;
	GtkWidget *frame;

	frame=gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_NONE);
	gtk_container_set_border_width(GTK_CONTAINER(frame),4);

	GtkWidget *profilepath=patheditor_new(&state.profilemanager);
	gtk_container_add(GTK_CONTAINER(frame),profilepath);

	gtk_widget_show(profilepath);
	gtk_widget_show(frame);

	label=gtk_label_new(_("ICC Profiles"));
	gtk_notebook_append_page(GTK_NOTEBOOK(book),frame,label);


	// Border path selector...

	PathDialogData dd;
	
	frame=gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_NONE);
	gtk_container_set_border_width(GTK_CONTAINER(frame),4);
	gtk_widget_show(frame);

	GtkWidget *hbox=gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);
	gtk_container_add(GTK_CONTAINER(frame),hbox);

	GtkWidget *borderpath=patheditor_new(&state.bordersearchpath);
	dd.pe=PATHEDITOR(borderpath);
	patheditor_get_paths(dd.pe,&dd.sp);

	gtk_box_pack_start(GTK_BOX(hbox),borderpath,TRUE,TRUE,0);
	gtk_widget_show(borderpath);

	GtkWidget *imagesel=imageselector_new(&dd.sp,false);
	dd.is=IMAGESELECTOR(imagesel);
	g_signal_connect(imagesel,"changed",G_CALLBACK(paths_changed),&dd);
	gtk_box_pack_start(GTK_BOX(hbox),imagesel,FALSE,FALSE,0);
	gtk_widget_show(imagesel);

	g_signal_connect(borderpath,"changed",G_CALLBACK(paths_changed),&dd);

	label=gtk_label_new(_("Borders"));
	gtk_notebook_append_page(GTK_NOTEBOOK(book),frame,label);

	// Background path selector...

	PathDialogData bgdd;

	frame=gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_NONE);
	gtk_container_set_border_width(GTK_CONTAINER(frame),4);
	gtk_widget_show(frame);

	hbox=gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);
	gtk_container_add(GTK_CONTAINER(frame),hbox);

	GtkWidget *bgpath=patheditor_new(&state.backgroundsearchpath);
	bgdd.pe=PATHEDITOR(bgpath);
	patheditor_get_paths(bgdd.pe,&bgdd.sp);

	gtk_box_pack_start(GTK_BOX(hbox),bgpath,TRUE,TRUE,0);
	gtk_widget_show(bgpath);

	imagesel=imageselector_new(&bgdd.sp,false);
	bgdd.is=IMAGESELECTOR(imagesel);
	g_signal_connect(imagesel,"changed",G_CALLBACK(paths_changed),&bgdd);
	gtk_box_pack_start(GTK_BOX(hbox),imagesel,FALSE,FALSE,0);
	gtk_widget_show(imagesel);

	g_signal_connect(bgpath,"changed",G_CALLBACK(paths_changed),&bgdd);

	label=gtk_label_new(_("Backgrounds"));
	gtk_notebook_append_page(GTK_NOTEBOOK(book),frame,label);


	gtk_widget_show(dialog);

	imageselector_refresh(IMAGESELECTOR(dd.is));
	imageselector_refresh(IMAGESELECTOR(bgdd.is));

	gint result=gtk_dialog_run(GTK_DIALOG(dialog));
	switch(result)
	{
		case GTK_RESPONSE_CANCEL:
			break;
		case GTK_RESPONSE_OK:
			patheditor_get_paths(PATHEDITOR(profilepath),&state.profilemanager);
			patheditor_get_paths(PATHEDITOR(borderpath),&state.bordersearchpath);
			patheditor_get_paths(PATHEDITOR(bgpath),&state.backgroundsearchpath);
			break;
	}
	gtk_widget_destroy(dialog);
}


char *ImageMask_Dialog(GtkWindow *parent,PhotoPrint_State &state,char *oldfn)
{
	char *nfn=oldfn;
	GtkWidget *dialog;
	dialog=gtk_dialog_new_with_buttons(_("Select a border..."),
		parent,GtkDialogFlags(0),
		GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
		GTK_STOCK_OK,GTK_RESPONSE_OK,
		NULL);
	gtk_window_set_default_size(GTK_WINDOW(dialog),250,350);

	GtkWidget *imagesel=imageselector_new(&state.bordersearchpath,true);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),imagesel,TRUE,TRUE,0);
	gtk_widget_show(imagesel);

	gtk_widget_show(dialog);

	imageselector_refresh(IMAGESELECTOR(imagesel));
	imageselector_set_filename(IMAGESELECTOR(imagesel),oldfn);

	gint result=gtk_dialog_run(GTK_DIALOG(dialog));
	switch(result)
	{
		case GTK_RESPONSE_CANCEL:
			break;
		case GTK_RESPONSE_OK:
			if(oldfn)
				free(oldfn);
			nfn=NULL;
			const char *nstr=imageselector_get_filename(IMAGESELECTOR(imagesel));
			if(nstr)
				nfn=strdup(nstr);
			break;
	}
	gtk_widget_destroy(dialog);
	return(nfn);
}


static void	profenable_changed(GtkWidget *widget,gpointer user_data)
{
	GtkWidget *psel=GTK_WIDGET(user_data);
	bool enabled=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	gtk_widget_set_sensitive(GTK_WIDGET(psel),enabled);
}


void ExportTiff_Dialog(GtkWindow *parent,PhotoPrint_State &state)
{
	GtkWidget *dialog;
	GtkWidget *hbox;
 	dialog=gtk_dialog_new_with_buttons(_("Export TIFF..."),
		parent,GtkDialogFlags(0),
		GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
		GTK_STOCK_OK,GTK_RESPONSE_OK,
		NULL);

	hbox=gtk_hbox_new(FALSE,5);
	GtkWidget *filechooser = gtk_file_chooser_widget_new (GTK_FILE_CHOOSER_ACTION_SAVE);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),hbox,TRUE,TRUE,5);
	gtk_box_pack_start(GTK_BOX(hbox),filechooser,TRUE,TRUE,5);
	gtk_widget_show(filechooser);
	gtk_widget_show(hbox);

	GtkWidget *table=gtk_table_new(2,3,FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),0);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),table,FALSE,FALSE,5);
	gtk_widget_show(table);

	// Filename

	GtkWidget *label;

	// Pages

	label=gtk_label_new(_("Page range:"));
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,1,2);
	gtk_widget_show(label);

	hbox=gtk_hbox_new(FALSE,0);

	GtkWidget *pagesentry=gtk_entry_new();
	gtk_entry_set_width_chars(GTK_ENTRY(pagesentry),7);
	gtk_box_pack_start(GTK_BOX(hbox),GTK_WIDGET(pagesentry),TRUE,TRUE,0);
	gtk_widget_show(pagesentry);

	// Resolution

	label=gtk_label_new(_("Resolution (dpi):"));
	gtk_box_pack_start(GTK_BOX(hbox),GTK_WIDGET(label),TRUE,TRUE,20);
	gtk_widget_show(label);

	GtkWidget *resspin=gtk_spin_button_new_with_range(72,2880,1);
	gtk_box_pack_start(GTK_BOX(hbox),GTK_WIDGET(resspin),TRUE,TRUE,0);
	gtk_widget_show(resspin);

	GtkWidget *deepcolour=gtk_check_button_new_with_label(_("Save as 16-bit:"));
	gtk_box_pack_start(GTK_BOX(hbox),GTK_WIDGET(deepcolour),TRUE,TRUE,20);
	gtk_widget_show(deepcolour);

	gtk_table_attach_defaults(GTK_TABLE(table),hbox,1,2,1,2);
	gtk_widget_show(hbox);

//	hbox=gtk_hbox_new(FALSE,5);

//	gtk_table_attach_defaults(GTK_TABLE(table),hbox,1,2,2,3);
//	gtk_widget_show(hbox);


	// Colour management

	GtkWidget *outputprof=profileselector_new(&state.profilemanager,IS_TYPE_NULL,true);
	gtk_table_attach_defaults(GTK_TABLE(table),outputprof,1,2,3,4);
	gtk_widget_show(outputprof);

	GtkWidget *profactive=gtk_check_button_new_with_label(_("Output Profile:"));
	g_signal_connect(G_OBJECT(profactive),"toggled",G_CALLBACK(profenable_changed),outputprof);
	gtk_table_attach_defaults(GTK_TABLE(table),profactive,0,1,3,4);
	gtk_widget_show(profactive);

#ifdef WIN32
	char *dirname=substitute_homedir("$HOME\\My Documents\\My Pictures");
#else
	char *dirname=substitute_homedir("$HOME");
#endif
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(filechooser),dirname);
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(filechooser),"exported.tif");
	free(dirname);

	gtk_entry_set_text(GTK_ENTRY(pagesentry),"1-");

	const char *fn=state.profilemanager.FindString("ExportProfile");
	if((!fn) || (strlen(fn)==0))
		fn=state.profilemanager.FindString("DefaultRGBProfile");
	if(fn && strlen(fn))
		profileselector_set_filename(PROFILESELECTOR(outputprof),fn);
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(profactive),state.profilemanager.FindInt("ExportProfileActive"));
	profenable_changed(profactive,outputprof);

	gtk_widget_show(dialog);
	
	bool done=false;
	while(!done)
	{
		gint result=gtk_dialog_run(GTK_DIALOG(dialog));
		switch(result)
		{
			case GTK_RESPONSE_CANCEL:
				done=true;
				break;
			case GTK_RESPONSE_OK:
				try
				{
					const char *profile=profileselector_get_filename(PROFILESELECTOR(outputprof));
					bool profileactive=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(profactive));
					const char *pagerange=gtk_entry_get_text(GTK_ENTRY(pagesentry));
					char *outputfilename=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(filechooser));
					int res=int(gtk_spin_button_get_value(GTK_SPIN_BUTTON(resspin)));
					bool save16bit=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(deepcolour));
		
					if(outputfilename)
					{
						struct stat statbuf;
						if(stat(outputfilename,&statbuf)==0)
						{
							GtkWidget *confirm = gtk_message_dialog_new (GTK_WINDOW(parent),GtkDialogFlags(0),
							GTK_MESSAGE_WARNING,GTK_BUTTONS_OK_CANCEL,
							"File exists - OK to overwrite?");
							gint response=gtk_dialog_run (GTK_DIALOG (confirm));
							cerr << "Response: " << response << endl;
							if(response!=GTK_RESPONSE_OK)
							{
								g_free(outputfilename);
								outputfilename=NULL;
							}
							gtk_widget_destroy (confirm);
						}
					}
					else
						ErrorMessage_Dialog(_("Please provide a filename."),GTK_WIDGET(parent));

					if(outputfilename)
					{				
						state.profilemanager.SetInt("ExportProfileActive",profileactive);
						if(profileactive)
							state.profilemanager.SetString("ExportProfile",profile);
		
						cerr << "Page range: " << pagerange << endl;
						cerr << "Resolution: " << res << endl;
						cerr << "Using profile?: " << profileactive << endl;
						if(profileactive)
						cerr << "  " << profile << endl;
		
						RangeParser rp(pagerange,state.layout->GetPages());
						int p;
						CMTransformFactory *factory=state.profilemanager.GetTransformFactory();
						while((p=rp.Next()))
						{
							cerr << "Exporting page " << p << " of " <<  state.layout->GetPages() << endl;
							cerr << "To filename... ";
							char *ftmp;
							if(state.layout->GetPages()>1)
								ftmp=SerialiseFilename(outputfilename,p,state.layout->GetPages());
							else
								ftmp=strdup(outputfilename);
							cerr << ftmp << endl;

							ImageSource *is=state.layout->GetImageSource(p-1,CM_COLOURDEVICE_EXPORT,factory,res,true);
							if(is)
							{
								ProgressBar p(_("Exporting..."),true,GTK_WIDGET(parent));

								if(profileactive)
								{
									CMSProfile *prof=state.profilemanager.GetProfile(CM_COLOURDEVICE_EXPORT);
									cerr << "Got profile " << prof->GetFilename() << endl;
									is->SetEmbeddedProfile(prof);
									cerr << "Set profile - saving..." << endl;
								}

								TIFFSaver ts(ftmp,is,save16bit);
								ts.SetProgress(&p);
								ts.Save();
								delete is;
							}
							
							free(ftmp);
						}
						delete factory;
						g_free(outputfilename);
						outputfilename=NULL;
						done=true;
					}
				}
				catch (const char *error)
				{
					ErrorMessage_Dialog(error,GTK_WIDGET(parent));
				}
				break;
		}
	}
	gtk_widget_destroy(dialog);
}


void ExportJPEG_Dialog(GtkWindow *parent,PhotoPrint_State &state)
{
	GtkWidget *dialog;
	GtkWidget *hbox;
 	dialog=gtk_dialog_new_with_buttons(_("Export JPEG..."),
		parent,GtkDialogFlags(0),
		GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
		GTK_STOCK_OK,GTK_RESPONSE_OK,
		NULL);
//	gtk_window_set_default_size(GTK_WINDOW(dialog),250,350);

	hbox=gtk_hbox_new(FALSE,5);
	GtkWidget *filechooser = gtk_file_chooser_widget_new (GTK_FILE_CHOOSER_ACTION_SAVE);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),hbox,TRUE,TRUE,5);
	gtk_box_pack_start(GTK_BOX(hbox),filechooser,TRUE,TRUE,5);
	gtk_widget_show(filechooser);
	gtk_widget_show(hbox);

	GtkWidget *table=gtk_table_new(2,5,FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),0);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),table,FALSE,FALSE,5);
	gtk_widget_show(table);

	// Filename

	GtkWidget *label;

	// Pages

	hbox=gtk_hbox_new(FALSE,0);

	label=gtk_label_new(_("Page range:"));
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,1,2);
	gtk_widget_show(label);

	GtkWidget *pagesentry=gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox),GTK_WIDGET(pagesentry),TRUE,TRUE,0);
	gtk_widget_show(pagesentry);

	// Resolution

	label=gtk_label_new(_("Resolution (dpi):"));
	gtk_box_pack_start(GTK_BOX(hbox),GTK_WIDGET(label),TRUE,TRUE,20);
	gtk_widget_show(label);

	GtkWidget *resspin=gtk_spin_button_new_with_range(72,2880,1);
	gtk_box_pack_start(GTK_BOX(hbox),GTK_WIDGET(resspin),TRUE,TRUE,0);
	gtk_widget_show(resspin);

	gtk_table_attach_defaults(GTK_TABLE(table),hbox,1,2,1,2);
	gtk_widget_show(hbox);


	// JPEG Quality

	hbox=gtk_hbox_new(FALSE,5);

	gtk_table_attach_defaults(GTK_TABLE(table),hbox,1,2,2,3);
	gtk_widget_show(hbox);

	label=gtk_label_new(_("JPEG Quality:"));
	gtk_misc_set_alignment(GTK_MISC(label),0.95,0.5);
	gtk_box_pack_start(GTK_BOX(hbox),GTK_WIDGET(label),TRUE,TRUE,0);
	gtk_widget_show(label);

	GtkWidget *qualspin=gtk_spin_button_new_with_range(60,100,1);
	gtk_box_pack_start(GTK_BOX(hbox),GTK_WIDGET(qualspin),FALSE,FALSE,0);
	gtk_widget_show(qualspin);


	// Colour management

	GtkWidget *outputprof=profileselector_new(&state.profilemanager,IS_TYPE_NULL,true);
	gtk_table_attach_defaults(GTK_TABLE(table),outputprof,1,2,4,5);
	gtk_widget_show(outputprof);

	GtkWidget *profactive=gtk_check_button_new_with_label(_("Output Profile:"));
	g_signal_connect(G_OBJECT(profactive),"toggled",G_CALLBACK(profenable_changed),outputprof);
	gtk_table_attach_defaults(GTK_TABLE(table),profactive,0,1,4,5);
	gtk_widget_show(profactive);

#ifdef WIN32
	char *dirname=substitute_homedir("$HOME\\My Documents\\My Pictures");
#else
	char *dirname=substitute_homedir("$HOME");
#endif
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(filechooser),dirname);
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(filechooser),"exported.jpg");
	free(dirname);

	gtk_entry_set_text(GTK_ENTRY(pagesentry),"1-");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(qualspin),85.0);

	const char *fn=state.profilemanager.FindString("ExportProfile");
	if((!fn) || (strlen(fn)==0))
		fn=state.profilemanager.FindString("DefaultRGBProfile");
	if(fn && strlen(fn))
		profileselector_set_filename(PROFILESELECTOR(outputprof),fn);
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(profactive),state.profilemanager.FindInt("ExportProfileActive"));
	profenable_changed(profactive,outputprof);

	gtk_widget_show(dialog);
	
	bool done=false;
	while(!done)
	{
		gint result=gtk_dialog_run(GTK_DIALOG(dialog));
		switch(result)
		{
			case GTK_RESPONSE_CANCEL:
				done=true;
				break;
			case GTK_RESPONSE_OK:
				try
				{
					const char *profile=profileselector_get_filename(PROFILESELECTOR(outputprof));
					bool profileactive=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(profactive));
					const char *pagerange=gtk_entry_get_text(GTK_ENTRY(pagesentry));
					const char *outputfilename=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(filechooser));
					int res=int(gtk_spin_button_get_value(GTK_SPIN_BUTTON(resspin)));
					int quality=int(gtk_spin_button_get_value(GTK_SPIN_BUTTON(qualspin)));
					if(outputfilename)
					{
						state.profilemanager.SetInt("ExportProfileActive",profileactive);
						if(profileactive)
							state.profilemanager.SetString("ExportProfile",profile);
		
						cerr << "Page range: " << pagerange << endl;
						cerr << "Resolution: " << res << endl;
						cerr << "Using profile?: " << profileactive << endl;
						if(profileactive)
						cerr << "  " << profile << endl;
		
						RangeParser rp(pagerange,state.layout->GetPages());
						int p;
						CMTransformFactory *factory=state.profilemanager.GetTransformFactory();
						while((p=rp.Next()))
						{
							cerr << "Exporting page " << p << " of " <<  state.layout->GetPages() << endl;
							cerr << "To filename... ";
							char *ftmp;
							if(state.layout->GetPages()>1)
								ftmp=SerialiseFilename(outputfilename,p,state.layout->GetPages());
							else
								ftmp=strdup(outputfilename);
							cerr << ftmp << endl;

							ImageSource *is=state.layout->GetImageSource(p-1,CM_COLOURDEVICE_EXPORT,factory,res,true);
							if(is)
							{
								ProgressBar p(_("Exporting..."),true,GTK_WIDGET(parent));

								if(profileactive)
								{
									CMSProfile *prof=state.profilemanager.GetProfile(CM_COLOURDEVICE_EXPORT);
									cerr << "Got profile " << prof->GetFilename() << endl;
									is->SetEmbeddedProfile(prof);
									cerr << "Set profile - saving..." << endl;
								}

								JPEGSaver ts(ftmp,is,quality);
								ts.SetProgress(&p);
								ts.Save();
								delete is;
							}
							
							free(ftmp);
						}
						delete factory;
						done=true;
					}
					else
						ErrorMessage_Dialog(_("Please provide a filename."),GTK_WIDGET(parent));
				}
				catch (const char *error)
				{
					ErrorMessage_Dialog(error,GTK_WIDGET(parent));
				}
				break;
		}
	}
	gtk_widget_destroy(dialog);
}


char *Background_Dialog(GtkWindow *parent,PhotoPrint_State &state,char *oldfn)
{
	char *nfn=oldfn;
	GtkWidget *dialog;
	dialog=gtk_dialog_new_with_buttons(_("Select a background..."),
		parent,GtkDialogFlags(0),
		GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
		GTK_STOCK_OK,GTK_RESPONSE_OK,
		NULL);
	gtk_window_set_default_size(GTK_WINDOW(dialog),250,350);

	GtkWidget *imagesel=imageselector_new(&state.backgroundsearchpath,true);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),imagesel,TRUE,TRUE,0);
	gtk_widget_show(imagesel);

	gtk_widget_show(dialog);

	imageselector_refresh(IMAGESELECTOR(imagesel));
	imageselector_set_filename(IMAGESELECTOR(imagesel),oldfn);

	gint result=gtk_dialog_run(GTK_DIALOG(dialog));
	switch(result)
	{
		case GTK_RESPONSE_CANCEL:
			break;
		case GTK_RESPONSE_OK:
			if(oldfn)
				free(oldfn);
			nfn=NULL;
			const char *nstr=imageselector_get_filename(IMAGESELECTOR(imagesel));
			if(nstr)
				nfn=strdup(nstr);
			break;
	}
	gtk_widget_destroy(dialog);
	return(nfn);
}


struct customprofstate
{
	Layout_ImageInfo *ii;
	GtkWidget *preview;
	GtkWidget *profselector;
	GtkWidget *profenabled;
	GtkWidget *intent;
	PhotoPrint_State *state;
	char *id;
};


static void refreshprofiledialog(GtkWidget *wid,gpointer obj)
{
	customprofstate *st=(customprofstate *)obj;
	const char *profile=profileselector_get_filename(PROFILESELECTOR(st->profselector));
	bool profileactive=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(st->profenabled));

	if(profile)
		profile=st->state->profilemanager.SearchPaths(profile);

	cerr << "Got profileactive: " << profileactive << endl;
	if(profile)
		cerr << "Got profile: " << profile << endl;

	if(profileactive)
		st->ii->AssignProfile(profile);
	else
		st->ii->AssignProfile(NULL);

	LCMSWrapper_Intent intent=intentselector_getintent(INTENTSELECTOR(st->intent));
	st->ii->SetRenderingIntent(intent);

	cerr << "Rendering intent: " << intent << endl;

	cerr << "Assigned" << endl;

	// FIXME - deal with Intent here.

	GdkPixbuf *tn=st->ii->GetThumbnail();
	cerr << "Drawing new image" << endl;
	gtk_image_set_from_pixbuf(GTK_IMAGE(st->preview),tn);
}


static void profiledialog_profilechanged(GtkWidget *wid,gpointer obj)
{
	customprofstate *st=(customprofstate *)obj;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(st->profenabled),true);
	refreshprofiledialog(wid,obj);
}


void SetCustomProfileDialog(GtkWindow *parent,PhotoPrint_State &state,Layout_ImageInfo *ii)
{
	if(!ii)
	{
		ErrorMessage_Dialog(_("Please select an image first!"),GTK_WIDGET(parent));
		return;
	}

	// Disable the second preview if there's no monitor or default profile!
	// (A more elegant solution might be to have the profile manager fall back
	// to the built-in sRGB profile if there's no default profile!)

	CMSProfile *targetprof;
	CMColourDevice target=CM_COLOURDEVICE_NONE;
	if((targetprof=state.profilemanager.GetProfile(CM_COLOURDEVICE_DISPLAY)))
		target=CM_COLOURDEVICE_DISPLAY;
	else if((targetprof=state.profilemanager.GetProfile(CM_COLOURDEVICE_DEFAULTRGB)))
		target=CM_COLOURDEVICE_DEFAULTRGB;
	if(targetprof)
		delete targetprof;

	// Store the original CustomProfile and intent; strdup filename.
	const char *octmp=ii->GetAssignedProfile();
	char *oldcustomprofile=NULL;
	if(octmp)
		oldcustomprofile=strdup(octmp);
	LCMSWrapper_Intent oldintent=ii->GetRenderingIntent();

	customprofstate st;
	st.ii=ii;
	st.state=&state;

	GtkWidget *dialog;
//	GtkWidget *hbox;
	GtkWidget *label;
 	dialog=gtk_dialog_new_with_buttons(_("Set Custom Profile..."),
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


	GdkPixbuf *tn1=ii->GetThumbnail();
	GtkWidget *image1 = gtk_image_new_from_pixbuf(tn1);
	GtkWidget *image2 = gtk_image_new_from_pixbuf(tn1);

	gtk_table_attach_defaults(GTK_TABLE(table),image1,0,1,1,2);
	gtk_widget_show(image1);

	if(target==CM_COLOURDEVICE_NONE)
	{
		label=gtk_label_new(_("Can't show a colour managed\npreview unless you choose\na Default RGB or Monitor profile\nin the Options->Colour Management\ndialog!"));
		gtk_table_attach_defaults(GTK_TABLE(table),label,1,2,1,2);
		gtk_widget_show(label);
	}
	else
	{
		gtk_table_attach_defaults(GTK_TABLE(table),image2,1,2,1,2);
		gtk_widget_show(image2);
	}

	st.preview=image2;

	table=gtk_table_new(2,3,FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_table_set_col_spacings(GTK_TABLE(table),0);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),table,FALSE,FALSE,5);
	gtk_widget_show(table);

	// Colour management

	ImageSource *is=ISLoadImage(ii->GetFilename());
	IS_TYPE t=IS_TYPE(STRIP_ALPHA(is->type));
	if(is->GetEmbeddedProfile())
	{
		GtkWidget *tmp=gtk_label_new(_("Warning: Image already has an embedded profile\nAssigning a new one will over-ride it!"));
		gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),tmp,FALSE,FALSE,5);
		gtk_widget_show(tmp);
	}
	delete is;

	// Analyse the image and fetch its colourspace.

	GtkWidget *outputprof=profileselector_new(&state.profilemanager,t,true);
	if(oldcustomprofile)
		profileselector_set_filename(PROFILESELECTOR(outputprof),oldcustomprofile);
	g_signal_connect(G_OBJECT(outputprof),"changed",G_CALLBACK(profiledialog_profilechanged),&st);
	gtk_table_attach_defaults(GTK_TABLE(table),outputprof,1,2,0,1);
	gtk_widget_show(outputprof);

	st.profselector=outputprof;

	GtkWidget *profactive=gtk_check_button_new_with_label(_("Assign Profile:"));
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(profactive),(oldcustomprofile!=NULL));
	g_signal_connect(G_OBJECT(profactive),"toggled",G_CALLBACK(refreshprofiledialog),&st);
	gtk_table_attach_defaults(GTK_TABLE(table),profactive,0,1,0,1);
	gtk_widget_show(profactive);

	st.profenabled=profactive;

	label=gtk_label_new(_("Rendering intent:"));
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,1,2);
	gtk_widget_show(label);

	st.intent = intentselector_new(&state.profilemanager);
	intentselector_setintent(INTENTSELECTOR(st.intent),oldintent);
	g_signal_connect(G_OBJECT(st.intent),"changed",G_CALLBACK(refreshprofiledialog),&st);
	gtk_table_attach_defaults(GTK_TABLE(table),st.intent,1,2,1,2);
	gtk_widget_show(st.intent);	

	gtk_widget_show(dialog);
	
	bool done=false;
	while(!done)
	{
		gint result=gtk_dialog_run(GTK_DIALOG(dialog));
		switch(result)
		{
			case GTK_RESPONSE_CANCEL:
				// restore original values...
				ii->AssignProfile(oldcustomprofile);
				ii->SetRenderingIntent(oldintent);
				done=true;
				break;
			case GTK_RESPONSE_OK:
				done=true;
				break;
		}
	}
	if(oldcustomprofile)
		free(oldcustomprofile);
	gtk_widget_destroy(dialog);
}


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
			cerr << "Failed to obtain imagesource for page" << page << endl;
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

