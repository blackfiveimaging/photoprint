/*
 * PhotoPrint
 * Copyright (c) 2004-2006 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <iostream>

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <gtk/gtk.h>
#include <gutenprint/gutenprint.h>

#include "config.h"
#include "gettext.h"


#include "support/configdb.h"
#include "photoprint_state.h"

#include "pp_mainwindow.h"
#include "support/progressbar.h"
#include "support/progresstext.h"
#include "dialogs.h"
#include "support/generaldialogs.h"
#include "splashscreen/splashscreen.h"
#include "profilemanager/profileselector.h"
#include "profilemanager/intentselector.h"
#include "support/patheditor.h"
#include "util.h"


#define _(x) gettext(x)
#define N_(x) gettext_noop(x)

using namespace std;

bool ParseOptions(int argc,char *argv[],char **presetname)
{
	int batchmode=false;
	static struct option long_options[] =
	{
		{"help",no_argument,NULL,'h'},
		{"version",no_argument,NULL,'v'},
		{"preset",required_argument,NULL,'p'},
		{"batch",no_argument,NULL,'b'},
		{0, 0, 0, 0}
	};

	while(1)
	{
		int c;
		c = getopt_long(argc,argv,"hvp:b",long_options,NULL);
		if(c==-1)
			break;
		switch (c)
		{
			case 'h':
				printf("Usage: %s [options] image1 [image2] ... \n",argv[0]);
				printf("\t -h --help\t\tdisplay this message\n");
				printf("\t -v --version\t\tdisplay version\n");
				printf("\t -p --preset\t\tread a specific preset file\n");
				printf("\t -b --batch\t\trun without user interface\n");
				throw 0;
				break;
			case 'v':
				printf("%s\n",PACKAGE_STRING);
				throw 0;
				break;
			case 'p':
				*presetname=optarg;
				break;
			case 'b':
				batchmode=true;
				break;
		}
	}
	return(batchmode);
}


static void destroy( GtkWidget *widget,
                     gpointer   data )
{
    gtk_main_quit ();
}


int main(int argc,char **argv)
{
	cerr << "Photoprint starting..." << endl;
	gboolean have_gtk=false;
	char *presetname=NULL;

	stp_init();
	cerr << "Initialized Gutenprint" << endl;

	bool batchmode=ParseOptions(argc,argv,&presetname);
	if(!batchmode)
		have_gtk=gtk_init_check (&argc, &argv);

	if(have_gtk)
		gtk_set_locale();
	else
		setlocale(LC_ALL,"");

	cerr << "Setting up gettext... " << PACKAGE << ", " << LOCALEDIR << endl;
	bindtextdomain(PACKAGE,LOCALEDIR);
	bind_textdomain_codeset(PACKAGE, "UTF-8");
	textdomain(PACKAGE);

	try
	{
		SplashScreen *splash=NULL;
		if(have_gtk)
		{
			splash=new SplashScreen;
			splash->SetMessage(_("Initializing..."));
		}

		PhotoPrint_State state(batchmode);

		cerr << "State created..." << endl;

		if(presetname)
			state.SetFilename(presetname);
		cerr << "Set initial filename" << endl;

		if(have_gtk)
			splash->SetMessage(_("Checking .photoprint directory..."));

		CheckSettingsDir(".photoprint");

		if(have_gtk)
			splash->SetMessage(_("Loading preset..."));
		state.ParseConfigFile();

		if(have_gtk)
		{
			splash->SetMessage(_("Creating layout..."));
			delete splash;
		}

		cerr << "Translation test: " << _("Initializing...") << endl;

		state.NewLayout();

		cerr << "Translation test: " << _("Initializing...") << endl;

		if(batchmode)
		{
			try
			{
				cerr << "Running in batch mode" << endl;
				if(argc>optind)
				{
					bool allowcropping=state.layoutdb.FindInt("AllowCropping");
					enum PP_ROTATION rotation=PP_ROTATION(state.layoutdb.FindInt("Rotation"));
					for(int i=optind;i<argc;++i)
					{
						cerr << "Adding file: " << argv[i] << endl;
						state.layout->AddImage(argv[i],allowcropping,rotation);
					}
					ProgressText p;
					state.layout->Print(&p);
				}
			}
			catch(const char *err)
			{
				cerr << "Error: " << err << endl;
			}
		}
		else
		{
			try
			{
				GtkWidget *mainwindow;
		cerr << "Translation test: " << _("Initializing...") << endl;
				mainwindow = pp_mainwindow_new(&state);
				g_signal_connect (G_OBJECT (mainwindow), "destroy",
					    G_CALLBACK (destroy), NULL);
				gtk_widget_show (mainwindow);

		cerr << "Translation test: " << _("Initializing...") << endl;
		
				if(argc>optind)
				{
					bool allowcropping=state.layoutdb.FindInt("AllowCropping");
					enum PP_ROTATION rotation=PP_ROTATION(state.layoutdb.FindInt("Rotation"));
					ProgressBar p(_("Loading images..."),true,mainwindow);
					int lastpage=0;
					for(int i=optind;i<argc;++i)
					{
						if(!p.DoProgress(i-optind,argc-optind))
							break;
						lastpage=state.layout->AddImage(argv[i],allowcropping,rotation);
					}
					state.layout->SetCurrentPage(lastpage);
				}
	
		cerr << "Translation test: " << _("Initializing...") << endl;

				pp_mainwindow_refresh(PP_MAINWINDOW(mainwindow));

		cerr << "Translation test: " << _("Initializing...") << endl;
	
				gtk_main ();
			}
			catch (const char *err)
			{
				ErrorMessage_Dialog(err);
			}
		}
	}
	catch(const char *err)
	{
		cerr << "Error: " << err << endl;
	}
	catch(int retcode)
	{
		return(retcode);
	}
	return(0);
}
