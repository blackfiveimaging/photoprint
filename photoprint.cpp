/*
 * PhotoPrint
 * Copyright (c) 2004-2009 by Alastair M. Robinson
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

#include "support/debug.h"
#include "support/configdb.h"
#include "photoprint_state.h"

#include "pp_mainwindow.h"
#include "support/progressbar.h"
#include "support/progresstext.h"
#include "dialogs.h"
#include "miscwidgets/generaldialogs.h"
#include "splashscreen/splashscreen.h"
#include "profilemanager/profileselector.h"
#include "profilemanager/intentselector.h"
#include "miscwidgets/patheditor.h"

#include "support/pathsupport.h"
#include "support/util.h"


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
		{"debug",required_argument,NULL,'d'},
		{0, 0, 0, 0}
	};

	while(1)
	{
		int c;
		c = getopt_long(argc,argv,"hvp:bd:",long_options,NULL);
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
				printf("\t -d --debug\t\tset debugging level - 0 for silent, 4 for verbose");
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
			case 'd':
				Debug.SetLevel(DebugLevel(atoi(optarg)));
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
	Debug[TRACE] << "Photoprint starting..." << endl;
	gboolean have_gtk=false;
	char *presetname=NULL;

	Debug.SetLevel(WARN);
#ifdef WIN32
	char *logname=substitute_homedir("$HOME" SEARCHPATH_SEPARATOR_S ".photoprint_errorlog");
	Debug.SetLogFile(logname);
	delete logname;
#endif

	bool batchmode=ParseOptions(argc,argv,&presetname);
	if(!batchmode)
		have_gtk=gtk_init_check (&argc, &argv);

	if(have_gtk)
		gtk_set_locale();
	else
		setlocale(LC_ALL,"");

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

		if(presetname)
			state.SetFilename(presetname);

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

		state.NewLayout();

		if(batchmode)
		{
			try
			{
				Debug[TRACE] << "Running in batch mode" << endl;
				if(argc>optind)
				{
					bool allowcropping=state.layoutdb.FindInt("AllowCropping");
					enum PP_ROTATION rotation=PP_ROTATION(state.layoutdb.FindInt("Rotation"));
					for(int i=optind;i<argc;++i)
					{
						Debug[TRACE] << "Adding file: " << argv[i] << endl;
						state.layout->AddImage(argv[i],allowcropping,rotation);
					}
					ProgressText p;
					state.layout->Print(&p);
				}
			}
			catch(const char *err)
			{
				Debug[ERROR] << "Error: " << err << endl;
			}
		}
		else
		{
			try
			{
				GtkWidget *mainwindow;
				mainwindow = pp_mainwindow_new(&state);
				g_signal_connect (G_OBJECT (mainwindow), "destroy",
					    G_CALLBACK (destroy), NULL);
				gtk_widget_show (mainwindow);

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
	
				pp_mainwindow_refresh(PP_MAINWINDOW(mainwindow));

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
		if(have_gtk)
			ErrorMessage_Dialog(err);
		Debug[ERROR] << "Error: " << err << endl;
	}
	catch(int retcode)
	{
		return(retcode);
	}
	return(0);
}
