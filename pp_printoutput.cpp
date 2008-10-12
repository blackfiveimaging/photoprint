// FIXME - need a destructor to clean up the printer queue strings.

#include <iostream>

#include <string.h>

#include <gtk/gtkframe.h>
#include <gtk/gtksizegroup.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkcontainer.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkstock.h>

#include "stpui_widgets/stpui_queue.h"
#include "stpui_widgets/stpui_printerselector.h"

#include "pp_printoutput.h"

#include "config.h"
#include "gettext.h"
#define _(x) gettext(x)

using namespace std;

enum {
	CHANGED_SIGNAL,
	LAST_SIGNAL
};

static guint pp_printoutput_signals[LAST_SIGNAL] = { 0 };

static void pp_printoutput_class_init (pp_PrintOutputClass *klass);
static void pp_printoutput_init (pp_PrintOutput *stpuicombo);


static void printersel_changed(GtkWidget *wid,gpointer *ob)
{
	cerr << "In printersel_changed()" << endl;
	pp_PrintOutput *lo=(pp_PrintOutput *)ob;

	const char *driver=stpui_printerselector_get_driver(STPUI_PRINTERSELECTOR(wid));
	if(driver)
	{
		lo->po->SetString("Driver",driver);

		g_signal_emit(G_OBJECT (ob),pp_printoutput_signals[CHANGED_SIGNAL], 0);
	}
}


static void pp_printoutput_queue_changed(GtkEntry *entry,gpointer *ud)
{
	cerr << "In pp_printoutput_queue_changed()" << endl;

	pp_PrintOutput *ob=PP_PRINTOUTPUT(ud);
	PrintOutput *po=ob->po;

	cerr << "Getting printer queue..." << endl;

	const char *val=po->GetPrinterQueue();
	if(val && strlen(val))
	{
		cerr << "Got printer queue: " << val << endl;
		char *driver=po->GetPrinterDriver();
		if(driver)
		{
			cerr << "Got driver: " << driver << " from Queue" << endl;
			po->SetString("Driver",driver);
			stpui_printerselector_set_driver(STPUI_PRINTERSELECTOR(ob->printersel),driver);
			free(driver);
		}
		g_signal_emit(G_OBJECT (ob),pp_printoutput_signals[CHANGED_SIGNAL], 0);
	}
}



void pp_printoutput_refresh(pp_PrintOutput *ob)
{
	cerr << "In pp_printoutput_refresh()" << endl;
	PrintOutput *po=ob->po;

	const char *driver=po->FindString("Driver");
	if(driver)
	{
		cerr << "Setting driver to " << driver << endl;
		stpui_printerselector_set_driver(STPUI_PRINTERSELECTOR(ob->printersel),driver);
	}
	const char *command=po->FindString("Command");
	if(command)
	{
		gtk_entry_set_text(GTK_ENTRY(ob->string),command);
	}
}


GtkWidget*
pp_printoutput_new (PrintOutput *po)
{
	pp_PrintOutput *ob=PP_PRINTOUTPUT(g_object_new (pp_printoutput_get_type (), NULL));
	gtk_box_set_spacing(GTK_BOX(ob),5);

	ob->po=po;

	GtkWidget *label;

	GtkWidget *table=gtk_table_new(2,3,FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_box_pack_start(GTK_BOX(ob),table,FALSE,FALSE,0);
	gtk_widget_show(table);

	label=gtk_label_new(_("Print Queue:"));
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,0,1);
	gtk_widget_show(label);

	cerr << "Calling DBToQueues()" << endl;

	po->DBToQueues();

	cerr << "Getting PQInfo" << endl;
	struct pqinfo *pq=po->GetPQInfo();
	cerr << "Building stpui_queue" << endl;
	ob->combo=stpui_queue_new(pq);
	cerr << "Done" << endl;
	gtk_table_attach_defaults(GTK_TABLE(table),ob->combo,1,2,0,1);
	gtk_widget_show(ob->combo);

	g_signal_connect(GTK_WIDGET(ob->combo),"changed",G_CALLBACK(pp_printoutput_queue_changed),ob);


	label=gtk_label_new(_("Printer Model:"));
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,1,2);
	gtk_widget_show(label);

	ob->printersel=stpui_printerselector_new();
	gtk_table_attach_defaults(GTK_TABLE(table),ob->printersel,1,2,1,2);
	g_signal_connect(G_OBJECT(ob->printersel),"changed",G_CALLBACK(printersel_changed),ob);

	stpui_printerselector_set_driver(STPUI_PRINTERSELECTOR(ob->printersel),po->FindString("Driver"));
	gtk_widget_show(ob->printersel);
	
	return(GTK_WIDGET(ob));
}


GType
pp_printoutput_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo pp_printoutput_info =
		{
			sizeof (pp_PrintOutputClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) pp_printoutput_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (pp_PrintOutput),
			0,
			(GInstanceInitFunc) pp_printoutput_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_VBOX, "pp_PrintOutput", &pp_printoutput_info, (GTypeFlags)0);
	}
	return stpuic_type;
}


static void
pp_printoutput_class_init (pp_PrintOutputClass *klass)
{
	pp_printoutput_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (klass),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (pp_PrintOutputClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
pp_printoutput_init (pp_PrintOutput *ob)
{
	ob->po=NULL;
}


void pp_printoutput_queue_dialog(PrintOutput *po)
{
	const char *oldqueue=po->FindString("Queue");
	char *labeltext=g_strdup_printf("The printer queue %s\n is not found - please choose another",oldqueue);
	GtkWidget *dlg=gtk_dialog_new_with_buttons("Printer queue not found",NULL,
		GtkDialogFlags(0),GTK_STOCK_OK,GTK_RESPONSE_OK,NULL);

	GtkWidget *vbox=gtk_vbox_new(FALSE,5);
		gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dlg)->vbox),vbox,TRUE,TRUE,8);
	gtk_widget_show(vbox);

	GtkWidget *label=gtk_label_new(labeltext);
	gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_CENTER);
	g_free(labeltext);
	gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(vbox),label,TRUE,TRUE,0);

	cerr << "Getting PQInfo" << endl;
	struct pqinfo *pq=po->GetPQInfo();
	cerr << "Building stpui_queue" << endl;
	GtkWidget *combo=stpui_queue_new(pq);
	gtk_widget_show(combo);
	gtk_box_pack_start(GTK_BOX(vbox),combo,FALSE,FALSE,8);

	gint result=gtk_dialog_run(GTK_DIALOG(dlg));
	switch(result)
	{
		case GTK_RESPONSE_OK:
			po->QueuesToDB();
			gtk_widget_destroy(dlg);
			break;
	}
}
