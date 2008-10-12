#include <iostream>

#include "gutenprint/gutenprint.h"
#include "stpui_optionbook.h"
#include "stpui_queue.h"
#include "../stp_support/printerqueues.h"

#include <gtk/gtk.h>

using namespace std;

#if 0

int main(int argc, char **argv)
{
	gtk_init(&argc,&argv);
	stp_init();

	GtkWidget *dialog=gtk_dialog_new_with_buttons("Printer Setup",
		NULL,GtkDialogFlags(0),
		GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
		GTK_STOCK_OK,GTK_RESPONSE_OK,
		NULL);
	gtk_window_set_default_size(GTK_WINDOW(dialog),500,350);

	pqinfo *pq=pqinfo_create();
	pq->SetCustomCommand(pq,"cat >/tmp/test.prn");

	GtkWidget *queue=stpui_queue_new(pq);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),queue,FALSE,FALSE,0);
	gtk_widget_show(queue);

	gtk_widget_show(dialog);
	gint result=gtk_dialog_run(GTK_DIALOG(dialog));
	switch(result)
	{
		case GTK_RESPONSE_CANCEL:
			break;
		case GTK_RESPONSE_OK:
			const char *cc=pq->GetCustomCommand(pq);
			if(cc)
				cerr << "Custom command: " << cc << endl;
			break;
	}
	gtk_widget_destroy(dialog);
	pq->Dispose(pq);

	return(0);
}

#else

int main(int argc, char **argv)
{
	gtk_init(&argc,&argv);
	stp_init();

	stp_vars_t *stpvars;
	stpvars=stp_vars_create();
	const stp_vars_t *defaults=stp_default_settings();
	stp_vars_copy(stpvars,defaults);
	stp_set_driver(stpvars,"ps");
	const stp_printer_t *printer=stp_get_printer(stpvars);
	if(printer)
	{
		cerr << "Setting defaults" << endl;
		stp_set_printer_defaults(stpvars,printer);
	}

	GtkWidget *dialog=gtk_dialog_new_with_buttons("Printer Setup",
		NULL,GtkDialogFlags(0),
		GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
		GTK_STOCK_OK,GTK_RESPONSE_OK,
		NULL);
	gtk_window_set_default_size(GTK_WINDOW(dialog),500,350);

	GtkWidget *optionbook=stpui_optionbook_new(stpvars,NULL,0);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),optionbook,TRUE,TRUE,0);
	gtk_widget_show(optionbook);

	gtk_widget_show(dialog);
	gint result=gtk_dialog_run(GTK_DIALOG(dialog));
	switch(result)
	{
		case GTK_RESPONSE_CANCEL:
			break;
		case GTK_RESPONSE_OK:
			break;
	}
	gtk_widget_destroy(dialog);

	if(stpvars)
		stp_vars_destroy(stpvars);
}

#endif
