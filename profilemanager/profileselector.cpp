/*
 * profileselector.c - provides a custom widget for selecting ICC profiles
 * given a SearchPath pointing to the profile locations.
 *
 * Copyright (c) 2006 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */


/* FIXME:  A destructor is needed for this class, to free up the option list.
   As it stands, a certain amount of memory will be lost when the widget is
   destroyed.
*/
#include <iostream>

#include <string.h>

#include <gtk/gtkentry.h>
#include <gtk/gtklist.h>
#include <gtk/gtkfilesel.h>
#include <gtk/gtkfilechooser.h>
#include <gtk/gtkmenuitem.h>

#include "generaldialogs.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)

#include "profileselector.h"

static const char *PS_ESCAPESTRING=N_("Other...");

using namespace std;

enum {
	CHANGED_SIGNAL,
	LAST_SIGNAL
};

static guint profileselector_signals[LAST_SIGNAL] = { 0 };

static void profileselector_class_init (ProfileSelectorClass *klass);
static void profileselector_init (ProfileSelector *sel);

class profsel_entry
{
	public:
	profsel_entry(const char *filename,const char *uiname) : filename(NULL), uiname(NULL)
	{
		this->filename=g_strdup(filename);
		this->uiname=g_strdup(uiname);
	}
	~profsel_entry()
	{
		if(filename)
			g_free(filename);
		if(uiname)
			g_free(uiname);
	}
	int filenamecmp(profsel_entry &a)
	{
		return(strcmp(a.filename,filename));
	}
	int uinamecmp(profsel_entry &a)
	{
		return(strcmp(a.uiname,uiname));
	}
	gchar *filename;
	gchar *uiname;
};


static int mycmp(const void *s1,const void *s2)
{
	profsel_entry *ps1=(profsel_entry *)s1;
	profsel_entry *ps2=(profsel_entry *)s2;
	return(ps1->filenamecmp(*ps2));
}

static int mycmp_desc(const void *s1,const void *s2)
{
	profsel_entry *ps1=(profsel_entry *)s1;
	profsel_entry *ps2=(profsel_entry *)s2;
	return(ps1->uinamecmp(*ps2));
}


static bool verifyprofile(ProfileSelector *c,ProfileInfo *pi)
{
	try
	{
		if(c->colourspace || (!c->allowdevicelink))
		{
			if(c->colourspace)
			{
				if(pi->GetColourSpace()!=c->colourspace)
					throw "Ignoring profile - wrong colourspace";
			}
			if(!c->allowdevicelink)
			{
				if(pi->IsDeviceLink())
					throw "Ignoring profile - can't use devicelinks";
			}
		}
		return(true);
	}
	catch(const char *err)
	{
//		cerr << "Profile Selector: " << err << endl;
	}
	return(false);
}


static void rebuild_menu(ProfileSelector *c)
{
	if(c->menu)
		gtk_option_menu_remove_menu(GTK_OPTION_MENU(c->optionmenu));
	c->menu=gtk_menu_new();

	// Build popup menu from optionlist...
	GList *iter=c->optionlist;
	while(iter)
	{
		profsel_entry *ps=(profsel_entry *)iter->data;
		gchar *text=ps->uiname;
		GtkWidget *menu_item = gtk_menu_item_new_with_label (text);
		gtk_menu_shell_append (GTK_MENU_SHELL (c->menu), menu_item);
		gtk_widget_show (menu_item);
		iter=g_list_next(iter);
	}	
	gtk_option_menu_set_menu(GTK_OPTION_MENU(c->optionmenu),c->menu);
}


static void profileselector_build_options(ProfileSelector *c)
{
	if(c->optionlist)
	{
		GList *element;
		element=c->optionlist;
		while(element)
		{
			g_free(element->data);
			element=g_list_next(element);
		}
		g_list_free(c->optionlist);
		c->optionlist=NULL;
	}

	ProfileInfo *pi=c->pm->GetFirstProfileInfo();
	while(pi)
	{
		const char *filename=pi->GetFilename();
		const char *uiname=pi->GetDescription();
		profsel_entry *ps=new profsel_entry(filename,uiname);
		if(!g_list_find_custom(c->optionlist,ps,mycmp))
		{
			if(verifyprofile(c,pi))
			{
				profsel_entry *ps2=new profsel_entry(filename,uiname);
				c->optionlist=g_list_append(c->optionlist,ps2);
			}
		}
		delete ps;
		pi=pi->Next();
	}

	c->optionlist=g_list_sort(c->optionlist,mycmp_desc);

	// "Other..." option, pops up a file selector.
	profsel_entry *otherps=new profsel_entry(PS_ESCAPESTRING,gettext(PS_ESCAPESTRING));
	c->optionlist=g_list_append(c->optionlist,otherps);

	rebuild_menu(c);
}


static void	profileselector_changed(GtkWidget *widget,gpointer user_data)
{
	ProfileSelector *c=PROFILESELECTOR(user_data);
	GtkOptionMenu *om=GTK_OPTION_MENU(c->optionmenu);

	gint index=gtk_option_menu_get_history(om);

	profsel_entry *ps=NULL;
	const char *val=NULL;
	if(c->optionlist)
		ps=(profsel_entry *)g_list_nth_data(c->optionlist,index);

	if(ps)
		val=ps->filename;

	if(val && strlen(val))
	{
		if(strcmp(val,PS_ESCAPESTRING)==0)
		{
			char *fn=File_Dialog("Choose ICC Profile...",NULL,NULL);
			if(fn)
			{
				profileselector_set_filename(c,fn);
				val=NULL;
			}
			else
				val=c->filename;
		}
		else if(index==c->currentidx)
		{
			// If the user has simply reselected the current profile
			// we don't want to trigger a signal.
			return;
		}

		if(val)
			profileselector_set_filename(c,val);

		g_signal_emit(G_OBJECT (c),
			profileselector_signals[CHANGED_SIGNAL], 0);
	}
}


GtkWidget*
profileselector_new (ProfileManager *pm,IS_TYPE colourspace,bool allowdevicelink)
{
	ProfileSelector *c=PROFILESELECTOR(g_object_new (profileselector_get_type (), NULL));

	c->pm=pm;
	c->colourspace=colourspace;
	c->allowdevicelink=allowdevicelink;

	c->optionmenu=gtk_option_menu_new();
	c->menu=NULL;  // Built on demand...

	c->tips=gtk_tooltips_new();
	gtk_tooltips_enable(c->tips);

	gtk_box_pack_start(GTK_BOX(c),GTK_WIDGET(c->optionmenu),TRUE,TRUE,0);

	gtk_widget_show(c->optionmenu);

	profileselector_build_options(c);
	g_signal_connect(c->optionmenu,"changed",G_CALLBACK(profileselector_changed),c);

	gint index=gtk_option_menu_get_history(GTK_OPTION_MENU(c->optionmenu));
	profsel_entry *pe=NULL;
	const char *val=NULL;
	if(c->optionlist)
		pe=(profsel_entry *)g_list_nth_data(c->optionlist,index);
	if(pe)
		val=pe->filename;
	if(val)
		profileselector_set_filename(c,val);
	
	return(GTK_WIDGET(c));
}


GType
profileselector_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo profileselector_info =
		{
			sizeof (ProfileSelectorClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) profileselector_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (ProfileSelector),
			0,
			(GInstanceInitFunc) profileselector_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_VBOX, "ProfileSelector", &profileselector_info, GTypeFlags(0));
	}
	return stpuic_type;
}


static void
profileselector_class_init (ProfileSelectorClass *klass)
{
	profileselector_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (klass),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (ProfileSelectorClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
profileselector_init (ProfileSelector *c)
{
	c->optionlist=NULL;
	c->filename=NULL;
}


gboolean profileselector_refresh(ProfileSelector *c)
{
	g_signal_handlers_block_matched (G_OBJECT (c->optionmenu), 
                                         G_SIGNAL_MATCH_DATA,
                                         0, 0, NULL, NULL, c);
	profileselector_build_options(c);
	g_signal_handlers_unblock_matched (G_OBJECT (c->optionmenu), 
                                         G_SIGNAL_MATCH_DATA,
                                         0, 0, NULL, NULL, c);
	return(true);
}


void profileselector_set_type(ProfileSelector *c,IS_TYPE colourspace)
{
	colourspace=IS_TYPE(STRIP_ALPHA(colourspace));
	IS_TYPE oldcolourspace=c->colourspace;
	if(colourspace!=IS_TYPE_NULL && (oldcolourspace!=colourspace))
	{
		c->colourspace=colourspace;
		profileselector_build_options(c);
		rebuild_menu(c);
		const char *def=NULL;
		CMSProfile *defp=c->pm->GetDefaultProfile(colourspace);
		if(defp)
		{
			if((def=defp->GetFilename()))
				profileselector_set_filename(c,def);
			delete defp;
		}
	}
}


void profileselector_set_filename(ProfileSelector *c,const char *filename)
{
	char *fn=c->pm->MakeRelative(filename);

	try
	{
		if(c->filename)
			free(c->filename);
		c->filename=fn; // Don't free fn, c->filename has "adopted" it.
		
		// If the filename is not present, add it.

		CMSProfile *tp=c->pm->GetProfile(c->filename);
		profsel_entry tempps(c->filename,tp ? tp->GetDescription() : _("Please choose a valid ICC profile"));
		delete tp;

		GList *node;
		if((node=g_list_find_custom(c->optionlist,&tempps,mycmp)))
		{
			gint i=g_list_position(c->optionlist,node);
			gtk_option_menu_set_history(GTK_OPTION_MENU(c->optionmenu),i);		
			c->currentidx=i;
		}
		else
		{
			g_signal_handlers_block_matched (G_OBJECT (c->optionmenu), 
		                                         G_SIGNAL_MATCH_DATA,
		                                         0, 0, NULL, NULL, c);

			profsel_entry *newps=new profsel_entry(c->filename,tempps.uiname);
			c->optionlist=g_list_append(c->optionlist,newps);
			rebuild_menu(c);
			gint i=g_list_length(c->optionlist)-1;
			gtk_option_menu_set_history(GTK_OPTION_MENU(c->optionmenu),i);
			c->currentidx=i;

			g_signal_handlers_unblock_matched (G_OBJECT (c->optionmenu), 
		                                         G_SIGNAL_MATCH_DATA,
		                                         0, 0, NULL, NULL, c);
		}

		gsize in,out;
		gchar *entry=g_locale_to_utf8(c->filename,-1,&in,&out,NULL);
		gtk_tooltips_set_tip(c->tips,c->optionmenu,entry,entry);
	}
	catch(const char *err)
	{
		cerr << "Profile Selector: " << err << endl;
	}
}


const char *profileselector_get_filename(ProfileSelector *p)
{
	return(p->filename);
}

