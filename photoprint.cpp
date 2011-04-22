/*
 * PhotoPrint
 * Copyright (c) 2004-2009 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <iostream>
#include <string>
#include <deque>

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <gtk/gtk.h>
#include <gutenprint/gutenprint.h>

#include "config.h"
#include "gettext.h"

#include "errordialogqueue.h"
#include "debug.h"
#include "configdb.h"
#include "photoprint_state.h"

#include "pp_mainwindow.h"
#include "progressbar.h"
#include "progresstext.h"
#include "breakhandler.h"
#include "dialogs.h"
#include "miscwidgets/generaldialogs.h"
#include "splashscreen/splashscreen.h"
#include "profilemanager/profileselector.h"
#include "profilemanager/intentselector.h"
#include "miscwidgets/patheditor.h"

#include "pathsupport.h"
#include "util.h"


#define _(x) gettext(x)
#define N_(x) gettext_noop(x)

using namespace std;

static int repeats;	// Should we have a BatchOptions class or something similar?

bool ParseOptions(int argc,char *argv[],std::deque<std::string> &presetnames)
{
	int batchmode=false;
	repeats=1;
	static struct option long_options[] =
	{
		{"help",no_argument,NULL,'h'},
		{"version",no_argument,NULL,'v'},
		{"preset",required_argument,NULL,'p'},
		{"batch",no_argument,NULL,'b'},
		{"repeat",required_argument,NULL,'r'},
		{"debug",required_argument,NULL,'d'},
		{0, 0, 0, 0}
	};

	while(1)
	{
		int c;
		c = getopt_long(argc,argv,"hvp:br:d:",long_options,NULL);
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
				printf("\t -r --repeat\t\trepeats each image n times\n");
				printf("\t -d --debug\t\tset debugging level - 0 for silent, 4 for verbose");
				throw 0;
				break;
			case 'v':
				printf("%s\n",PACKAGE_STRING);
				throw 0;
				break;
			case 'p':
				Debug[TRACE] << "Adding preset file " << optarg << endl;
				presetnames.push_back(optarg);
				break;
			case 'b':
				batchmode=true;
				break;
			case 'r':
				repeats=atoi(optarg);
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



class PreloadProfiles : public Job
{
	public:
	PreloadProfiles(ProfileManager &pm) : Job("Finding ICC profiles..."), pm(pm)
	{
	}
	void Run(Worker *w)
	{
		Debug[TRACE] << "Getting profile list..." << endl;
		pm.ObtainMutex();
		ProfileInfo *pi=pm.GetFirstProfileInfo();
		while(pi)
		{
			try
			{
				pi->GetColourSpace();	// Trigger loading and caching of profile info
			}
			catch(const char *err)
			{
				Debug[WARN] << "Error: " << err << std::endl;
			}
			pi=pi->Next();
		}		
		pm.ReleaseMutex();
	}
	protected:
	ProfileManager &pm;
};


int main(int argc,char **argv)
{
	Debug[TRACE] << "Photoprint starting..." << endl;
	gboolean have_gtk=false;
	std::deque<string> presetnames;

	Debug.SetLevel(WARN);
#ifdef WIN32
	char *logname=substitute_homedir("$HOME" SEARCHPATH_SEPARATOR_S ".photoprint_errorlog");
	Debug.SetLogFile(logname);
	free(logname);
#endif

	try
	{
		bool batchmode=ParseOptions(argc,argv,presetnames);
//		if(!batchmode)	// We'll try an initialise GTK even if we're in batchmode, so we can use gdkpixbuf loaders, etc.
		have_gtk=gtk_init_check (&argc, &argv);

		if(have_gtk)
			gtk_set_locale();
		else
		{
			g_type_init();
			setlocale(LC_ALL,"");
		}

		bindtextdomain(PACKAGE,LOCALEDIR);
		bind_textdomain_codeset(PACKAGE, "UTF-8");
		textdomain(PACKAGE);

		SplashScreen *splash=NULL;
		if(have_gtk)
		{
			splash=new SplashScreen;
			splash->SetMessage(_("Initializing..."));
		}

		PhotoPrint_State state(batchmode);

		if(have_gtk)
			splash->SetMessage(_("Checking .photoprint directory..."));

		CheckSettingsDir(".photoprint");

		if(have_gtk)
			splash->SetMessage(_("Loading preset..."));
		state.ParseConfigFile();
		for(unsigned int i=0;i<presetnames.size();++i)
		{
			Debug[TRACE] << "Parsing config file: " << presetnames[i] << endl;
			state.SetFilename(presetnames[i].c_str());
			state.ParseConfigFile();
		}

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
						for(int j=0;j<repeats;++j)
							state.layout->AddImage(argv[i],allowcropping,rotation);
					}
					ProgressTextBreak p;
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
					ProgressBar p(_("Loading images..."),true,mainwindow);
					int lastpage=0;
					for(int i=optind;i<argc;++i)
					{
						if(!p.DoProgress(i-optind,argc-optind))
							break;
						for(int j=0;j<repeats;++j)
							lastpage=state.layout->AddImage_Defaults(argv[i]);
					}
					state.layout->SetCurrentPage(lastpage);
				}
	
				pp_mainwindow_refresh(PP_MAINWINDOW(mainwindow));
	
				JobDispatcher disp(1);
				disp.AddJob(new PreloadProfiles(state.profilemanager));
				gtk_main ();
			}
			catch (const char *err)
			{
				ErrorDialogs.AddMessage(err);
//				ErrorMessage_Dialog(err);
			}
		}
	}
	catch(const char *err)
	{
		ErrorDialogs.AddMessage(err);
//		if(have_gtk)
//			ErrorMessage_Dialog(err);
		Debug[ERROR] << "Error: " << err << endl;
	}
	catch(int retcode)
	{
		return(retcode);
	}
	return(0);
}
