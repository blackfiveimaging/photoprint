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
#include "pp_printoutput.h"
#include "pp_cms.h"
#include "pp_units.h"
#include "pp_scaling.h"
#include "stpui_widgets/stpui_optionbook.h"
#include "splashscreen/splashscreen.h"
#include "support/patheditor.h"
#include "support/pathsupport.h"
#include "support/imageselector.h"
#include "support/generaldialogs.h"
#include "support/rangeparser.h"
#include "support/progressbar.h"
#include "support/tiffsave.h"
#include "support/jpegsave.h"
#include "profilemanager/profileselector.h"
#include "profilemanager/intentselector.h"
#include "util.h"
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
		pp_printoutput_refresh(PP_PRINTOUTPUT(cbd->printoutput));
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

	dialogdata.printoutput=pp_printoutput_new(&state.printoutput);
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
	
	GtkWidget *label=gtk_label_new("PhotoPrint "
	PACKAGE_VERSION
	" - copyright (c) 2004-2008\n"
	"by Alastair M. Robinson (amr@blackfiveservices.co.uk)\nDistributed under the terms\nof the GNU General Public Licence.\n"
	"See the file 'COPYING' for more details.");
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

							ImageSource *is=state.layout->GetImageSource(p-1,CM_COLOURDEVICE_EXPORT,factory,res);
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

							ImageSource *is=state.layout->GetImageSource(p-1,CM_COLOURDEVICE_EXPORT,factory,res);
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
		profile=st->state->profilemanager.Search(profile);

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
