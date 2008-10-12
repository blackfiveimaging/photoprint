#include <string.h>

#include <gtk/gtklist.h>
#include <gtk/gtkentry.h>

#include "stpui_printerselector.h"

enum {
	CHANGED_SIGNAL,
	LAST_SIGNAL
};

static guint stpui_printerselector_signals[LAST_SIGNAL] = { 0 };

static void stpui_printerselector_class_init (stpui_PrinterSelectorClass *klass);
static void stpui_printerselector_init (stpui_PrinterSelector *stpuicombo);


static void stpui_printerselector_build_manufacturers(stpui_PrinterSelector *c)
{
	int count,j;

	if(c->manufacturercount)
	{
		gtk_list_clear_items(GTK_LIST(GTK_COMBO(c->mancombo)->list),0,-1);
		c->manufacturercount=0;
		g_list_free(c->manufacturers);
		c->manufacturers=NULL;
	}

	count=stp_printer_model_count();
	
	for(j=0;j<count;++j)
	{
		const stp_printer_t *printer=stp_get_printer_by_index(j);
		const char *manufacturer=stp_printer_get_manufacturer(printer);
		if(strlen(manufacturer))
		{
			if(!g_list_find_custom(c->manufacturers,manufacturer,(GCompareFunc)strcmp))
			{
				c->manufacturers=g_list_append(c->manufacturers,(void *)manufacturer);
				++c->manufacturercount;
			}
		}
	}
	
	if(c->manufacturers)
		gtk_combo_set_popdown_strings(GTK_COMBO(c->mancombo),c->manufacturers);
	if(c->manufacturer)
		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(c->mancombo)->entry),c->manufacturer);
}


static void stpui_printerselector_build_models(stpui_PrinterSelector *c)
{
	int count,j;
	const char *defmodel=NULL;

	if(c->modelcount)
	{
		gtk_list_clear_items(GTK_LIST(GTK_COMBO(c->modelcombo)->list),0,-1);
		c->modelcount=0;
		g_list_free(c->models);
		c->models=NULL;
	}

	count=stp_printer_model_count();
	
	for(j=0;j<count;++j)
	{
		const stp_printer_t *printer=stp_get_printer_by_index(j);
		const char *manufacturer=stp_printer_get_manufacturer(printer);
		if(strlen(manufacturer))
		{
			if(strcmp(manufacturer,c->manufacturer)==0)
			{
				const char *model=stp_printer_get_long_name(printer);
				c->models=g_list_append(c->models,(void *)model);
				if(c->modelcount==0 || (strcmp(c->model,model)==0))
					defmodel=model;
				++c->modelcount;
			}
		}
	}
	
	if(c->models)
	{
		gtk_combo_set_popdown_strings(GTK_COMBO(c->modelcombo),c->models);
	
		if(c->model)
		{
			g_free(c->model);
			c->model=NULL;
		}
	
		if(defmodel)
		{
			c->model=g_strdup(defmodel);
			gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(c->modelcombo)->entry),c->model);
		}
	}
}


static void stpui_printerselector_man_changed(GtkEntry *entry,gpointer *ud)
{
	stpui_PrinterSelector *c=STPUI_PRINTERSELECTOR(ud);
	const char *val=gtk_entry_get_text(entry);
	if(val && strlen(val))
	{
		if(c->manufacturer)
			g_free(c->manufacturer);
		c->manufacturer=g_strdup(val);
		
		stpui_printerselector_build_models(c);
	}
}


static int verifymodel(const char *model)
{
	int result=FALSE;
	if(stp_get_printer_by_long_name(model))
		result=TRUE;
	return(result);
}


static void stpui_printerselector_model_changed(GtkEntry *entry,gpointer *ud)
{
	stpui_PrinterSelector *c=STPUI_PRINTERSELECTOR(ud);
	const char *val=gtk_entry_get_text(entry);

	if(val && strlen(val))
	{
		if(verifymodel(val))
		{
			if(c->model)
				g_free(c->model);
			c->model=g_strdup(val);
			
			g_signal_emit(G_OBJECT (c),
				stpui_printerselector_signals[CHANGED_SIGNAL], 0);
		}
	}
}


GtkWidget*
stpui_printerselector_new ()
{
	stpui_PrinterSelector *c=STPUI_PRINTERSELECTOR(g_object_new (stpui_printerselector_get_type (), NULL));

	c->mancombo=gtk_combo_new();
	gtk_combo_set_value_in_list(GTK_COMBO(c->mancombo),TRUE,FALSE);

	c->modelcombo=gtk_combo_new();
	gtk_combo_set_value_in_list(GTK_COMBO(c->modelcombo),TRUE,FALSE);

	stpui_printerselector_build_manufacturers(c);
	stpui_printerselector_build_models(c);

	g_signal_connect(GTK_COMBO(c->mancombo)->entry,"changed",G_CALLBACK(stpui_printerselector_man_changed),c);
	g_signal_connect(GTK_COMBO(c->modelcombo)->entry,"changed",G_CALLBACK(stpui_printerselector_model_changed),c);

	gtk_box_pack_start(GTK_BOX(c),GTK_WIDGET(c->mancombo),TRUE,TRUE,0);
	gtk_widget_show(c->mancombo);
	gtk_box_pack_start(GTK_BOX(c),GTK_WIDGET(c->modelcombo),TRUE,TRUE,0);
	gtk_widget_show(c->modelcombo);

	return(GTK_WIDGET(c));
}


GType
stpui_printerselector_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo stpui_printerselector_info =
		{
			sizeof (stpui_PrinterSelectorClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) stpui_printerselector_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (stpui_PrinterSelector),
			0,
			(GInstanceInitFunc) stpui_printerselector_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_HBOX, "stpui_PrinterSelector", &stpui_printerselector_info, 0);
	}
	return stpuic_type;
}


static void
stpui_printerselector_class_init (stpui_PrinterSelectorClass *klass)
{
	stpui_printerselector_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET (stpui_PrinterSelectorClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
stpui_printerselector_init (stpui_PrinterSelector *c)
{
	c->manufacturercount=0;
	c->manufacturers=NULL;
	c->manufacturer=g_strdup("Epson");
	c->modelcount=0;
	c->models=NULL;
	c->model=g_strdup("Stylus Color 600");
}


const char *stpui_printerselector_get_printer_name(stpui_PrinterSelector *c)
{
	return(c->model);
}


const char *stpui_printerselector_get_driver(stpui_PrinterSelector *c)
{
	const stp_printer_t *printer=stp_get_printer_by_long_name(c->model);
	const char *driver=stp_printer_get_driver(printer);
	return(driver);
}


void stpui_printerselector_set_printer_name(stpui_PrinterSelector *c,const char *name)
{
	const stp_printer_t *printer=stp_get_printer_by_long_name(name);
	
	if(printer)
	{
		c->manufacturer=strdup(stp_printer_get_manufacturer(printer));
		c->model=strdup(name);
		stpui_printerselector_build_models(c);
		if(c->manufacturer)
		{
			gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(c->mancombo)->entry),c->manufacturer);
		}
	}
}


void stpui_printerselector_set_driver(stpui_PrinterSelector *c,const char *driver)
{
	if(driver)
	{
		const stp_printer_t *printer=stp_get_printer_by_driver(driver);
		if(printer)
		{
			const char *name=stp_printer_get_long_name(printer);
			printf("Found driver %s\n",driver);
			g_signal_handlers_block_matched (G_OBJECT(GTK_COMBO(c->modelcombo)->entry),G_SIGNAL_MATCH_DATA,
				0, 0, NULL, NULL, c);
			g_signal_handlers_block_matched (G_OBJECT(GTK_COMBO(c->mancombo)->entry),G_SIGNAL_MATCH_DATA,
				0, 0, NULL, NULL, c);
			stpui_printerselector_set_printer_name(c,name);
			g_signal_handlers_unblock_matched (G_OBJECT(GTK_COMBO(c->modelcombo)->entry),G_SIGNAL_MATCH_DATA,
				0, 0, NULL, NULL, c);
			g_signal_handlers_unblock_matched (G_OBJECT(GTK_COMBO(c->mancombo)->entry),G_SIGNAL_MATCH_DATA,
				0, 0, NULL, NULL, c);
		}
		else
			g_print("Can't find driver %s\n",driver);
	}
	else
		g_print("No driver supplied!\n");
}
