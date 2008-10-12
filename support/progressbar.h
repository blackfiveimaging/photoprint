#ifndef PP_PROGRESS_H
#define PP_PROGRESS_H

#include "progress.h"
#include <gtk/gtkwidget.h>

class ProgressBar : public Progress
{
	public:
	ProgressBar(const char *message,bool cancelbutton,GtkWidget *parent=NULL);
	~ProgressBar();
	bool DoProgress(int i,int maxi);
	void SetMessage(const char *msg);
	static void cancel_callback(GtkWidget *wid,gpointer *ob);
	private:
	char *message;
	GtkWidget *window;
	GtkWidget *progressbar;
	GtkWidget *label;
	bool cancelled;
};

#endif
