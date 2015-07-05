#include <iostream>
#include <string>
#include <algorithm>
#include <vector>

#include <libgen.h>

#include "config.h"

#include <gtk/gtkstock.h>
#include <gtk/gtkmain.h>

#include "pp_menu_shortcuts.h"
#include "pp_mainwindow.h"
#include "dialogs.h"
#include "miscwidgets/generaldialogs.h"

#include "debug.h"
#include "progressbar.h"
#include "pathsupport.h"
#include "searchpath.h"
#include "dirtreewalker.h"
#include "configdb.h"

#include "gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)

using namespace std;


class ShortcutMenuItem : public ConfigFile, public ConfigDB
{
	public:
	ShortcutMenuItem(string path,GtkActionGroup *group, pp_MainWindow *mw) : ConfigDB(Template), path(path), mw(mw)
	{
		// Extract the display name from the shortcut...
		new ConfigDBHandler(this,"[Shortcut]",this);
		ParseConfigFile(path.c_str());

		Debug[TRACE] << "Have menu item: " << FindString("DisplayName") << endl;
		action.name=action.label=FindString("DisplayName");
		action.stock_id=NULL;
		action.accelerator=NULL;
		action.tooltip=FindString("ToolTip");
		action.callback=G_CALLBACK(selected);

		gtk_action_group_add_actions_full(group,&action,1,this,destroy);
	}
	~ShortcutMenuItem()
	{
	}
	static void selected(GtkAction *action,gpointer ob)
	{
		ShortcutMenuItem *sm=(ShortcutMenuItem *)ob;
		sm->mw->state->ParseSupplementaryConfig(sm->path.c_str());
		pp_mainwindow_rebuild(sm->mw);
	}
	static void destroy(gpointer ob)
	{
		ShortcutMenuItem *sm=(ShortcutMenuItem *)ob;
		delete sm;
	}
	protected:
	string path;
	pp_MainWindow *mw;
	GtkActionEntry action;
	ConfigFile config;
	static ConfigTemplate Template[];
};

ConfigTemplate ShortcutMenuItem::Template[]=
{
	ConfigTemplate("DisplayName",""),
	ConfigTemplate("ToolTip",""),
	ConfigTemplate()
};


static GtkActionEntry shortcutsmenu_entries[] = {
  { "ShortcutsMenu", NULL, N_("_Shortcuts") }
};


// Class to maintain a list of strings, disallowing duplicates.

class shortcutlist : public std::vector<std::string>
{
	public:
	shortcutlist() : std::vector<std::string>()
	{
	}
	~shortcutlist()
	{
	}
	bool exists(std::string &str)
	{
		Debug[TRACE] << "Hunting for " << str << endl;
		for(unsigned int i=0;i<size();++i)
		{
			if((*this)[i]==str)
				return(true);
		}
		return(false);
	}
	// Adds the basename of the given string, stripping any path component.
	bool addbase(std::string str)
	{
		char *tmp=strdup(str.c_str());
		char *t2=basename(tmp);
		std::string str2(t2);
		free(tmp);
		return(add(str2));
	}
	// Returns true if the item was added, false if it already existed
	bool add(std::string str)
	{
		if(!exists(str))
		{
			Debug[TRACE] << "Adding " << str << endl;
			push_back(str);
			return(true);
		}
		return(false);
	}	
	void sortlist()
	{
		sort(begin(),end());
	}
};


void BuildShortcutsMenu(void *userdata,GtkUIManager *ui_manager)
{
	pp_MainWindow *mw=(pp_MainWindow *)userdata;	

	GError *error=NULL;
	GtkActionGroup *action_group;
	action_group = gtk_action_group_new ("ShortcutsMenuActions");
	gtk_action_group_set_translation_domain(action_group,PACKAGE);

	// Now we scan for shortcuts...
	const char *path=mw->state->FindString("ShortcutsPath");
	SearchPathHandler sp;
	sp.AddPath(path);
	SearchPathIterator spi(sp);

	string uidesc="<ui><menubar name='MainMenu'><menu action='ShortcutsMenu'>\n";
	gtk_action_group_add_actions (action_group, shortcutsmenu_entries, G_N_ELEMENTS (shortcutsmenu_entries), userdata);

	shortcutlist globallist;

	path=NULL;
	while((path=spi.GetNextPath(path)))
	{
		// For each path in turn we scan for files...
		DirTreeWalker dtw(path);
		DirTreeWalker *dir=dtw.NextDirectory();
		bool separator=false;
		while(dir)
		{
			shortcutlist strlist;
			const char *file;

			while((file=dir->NextFile()))
			{
				if(globallist.addbase(file))
					strlist.add(file);
				else
					Debug[WARN] << "Not adding " << file << " since an entry of that name already exists." << std::endl;
			}
			strlist.sortlist();

			vector<string>::const_iterator it;
			for(it=strlist.begin(); it!=strlist.end(); ++it)
			{
				if(separator)
				{
					uidesc+="<separator/>\n";
					separator=false;
				}
				// Now we have a list of files in alphabetical order, extract display names
				// and construct menu items...
				ShortcutMenuItem *item=new ShortcutMenuItem(*it,action_group,mw);
				uidesc+="<menuitem action='";
				const char *dname=item->FindString("DisplayName");
				if(!dname)
					dname=_("Error fetching preset name!");
				uidesc+=dname;
				uidesc+="'/>\n";
			}
			separator=true;
			dir=dir->NextDirectory();
		}
	}

	uidesc+="</menu></menubar></ui>";


	gtk_ui_manager_insert_action_group (ui_manager, action_group, 0);
	
	if (!gtk_ui_manager_add_ui_from_string (ui_manager, uidesc.c_str(), -1, &error))
		throw error->message;
}

