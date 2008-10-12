#include <string.h>

#include <gtk/gtkframe.h>
#include <gtk/gtkexpander.h>
#include <gtk/gtksizegroup.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkscrolledwindow.h>

#include "config.h"

#include "support/thread.h"
#include "stpui_widgets/dimension.h"
#include "imagesource/imagesource_util.h"

#include "photoprint_state.h"

#include "pp_imageinfo.h"

#include "gettext.h"
#define _(x) gettext(x)

enum {
	CHANGED_SIGNAL,
	LAST_SIGNAL
};

static guint pp_imageinfo_signals[LAST_SIGNAL] = { 0 };

static void pp_imageinfo_class_init (pp_ImageInfoClass *klass);
static void pp_imageinfo_init (pp_ImageInfo *stpuicombo);


void pp_imageinfo_refresh(pp_ImageInfo *ob)
{
	cerr << "Refreshing imageinfo" << endl;
	if(ob->layout)
	{
		Layout_ImageInfo *ii=ob->layout->FirstSelected();
		if(ii)
		{
			float pixelwidth=ii->GetWidth();
			float pixelheight=ii->GetHeight();
			cerr << "Image dimensions: " << pixelwidth << " x " << pixelheight << endl;
			RectFit *fit=ii->GetFit(1.0);
			cerr << "Got fit" << endl;
			if(fit)
			{
				double w=fit->width;
				double h=fit->height;
				float t;
				switch(fit->rotation)
				{
					case 90:
					case 270:
						cerr << "Rotation - swapping pixel dimensions" << endl;
						t=pixelwidth;
						pixelwidth=pixelheight;
						pixelheight=t;
						break;
					default:
						break;
				}

				LayoutRectangle *bounds=ii->GetBounds();
				if(bounds)
				{
					if(w>bounds->w)
					{
						cerr << "Cropping " << w << " to " << bounds->w << endl;
						pixelwidth=(bounds->w*pixelwidth)/w;
						cerr << "Pixelwidth reduced to: " << pixelwidth << endl;
						w=bounds->w;
					}
					if(h>bounds->h)
					{
						cerr << "Cropping " << h << " to " << bounds->h << endl;
						pixelheight=(bounds->h*pixelheight)/h;
						cerr << "Pixelheight reduced to: " << pixelheight << endl;
						h=bounds->h;
					}

					// Calculate DPI while units are still in points!
					float xres=pixelwidth/(w/72.0);
					float yres=pixelheight/(h/72.0);

					gchar *label=g_strdup_printf("%d x %d dpi",int(xres),int(yres));
					gtk_label_set_label(GTK_LABEL(ob->resolution),label);
					g_free(label);


					// Convert units from points to display unit.

					const char *unit="??";
					switch(ii->layout.state.GetUnits())
					{
						case UNIT_INCHES:
							w=UNIT_POINTS_TO_INCHES(w);
							h=UNIT_POINTS_TO_INCHES(h);
							unit=_("in");
							break;
						case UNIT_POINTS:
							unit=_("pt");
							break;
						case UNIT_CENTIMETERS:
							w=UNIT_POINTS_TO_CENTIMETERS(w);
							h=UNIT_POINTS_TO_CENTIMETERS(h);
							unit=_("cm");
							break;
						case UNIT_MILLIMETERS:
							w=UNIT_POINTS_TO_MILLIMETERS(w);
							h=UNIT_POINTS_TO_MILLIMETERS(h);
							unit=_("mm");
							break;
					}
					label=g_strdup_printf("%.1f x %.1f %s",w,h,unit);
					gtk_label_set_label(GTK_LABEL(ob->physicalsize),label);
					g_free(label);

					delete bounds;
				}
				delete fit;
			}
		}
	}
}


static void killshadow(GtkWidget *wid,gpointer data)
{
	gtk_viewport_set_shadow_type(GTK_VIEWPORT(wid),GTK_SHADOW_NONE);
}


GtkWidget*
pp_imageinfo_new (Layout *layout)
{
	pp_ImageInfo *ob=PP_IMAGEINFO(g_object_new (pp_imageinfo_get_type (), NULL));
//	gtk_box_set_spacing(GTK_BOX(ob),5);
	ob->layout=layout;
	GtkWidget *label;
//	GtkWidget *frame;
	
	// Layout frame

	gtk_expander_set_label(GTK_EXPANDER(ob),_("Image Info"));
	gtk_expander_set_spacing(GTK_EXPANDER(ob),5);
	gtk_expander_set_expanded(GTK_EXPANDER(ob),true);

//	gtk_box_pack_start(GTK_BOX(ob),frame,FALSE,FALSE,0);
//	gtk_widget_show(frame);

	ob->scrollwin=gtk_scrolled_window_new(NULL,NULL);
	gtk_widget_show(ob->scrollwin);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (ob->scrollwin),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
	gtk_container_add(GTK_CONTAINER(ob),ob->scrollwin);

	ob->table=gtk_table_new(2,5,false);

	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (ob->scrollwin), ob->table);
	gtk_container_foreach(GTK_CONTAINER(ob->scrollwin),killshadow,NULL);

	gtk_table_set_col_spacings(GTK_TABLE(ob->table),8);
	gtk_table_set_row_spacings(GTK_TABLE(ob->table),4);

	label=gtk_label_new(_("Filename:"));
	gtk_misc_set_alignment(GTK_MISC(label),1.0,0.5);
	gtk_table_attach(GTK_TABLE(ob->table),GTK_WIDGET(label),0,1,0,1,
		GtkAttachOptions(GTK_SHRINK|GTK_FILL),GtkAttachOptions(GTK_SHRINK|GTK_FILL),0,0);
	gtk_widget_show(label);

	ob->filename=gtk_label_new("");
	gtk_misc_set_alignment(GTK_MISC(ob->filename),0.0,0.5);
	gtk_widget_show(ob->filename);

	gtk_table_attach_defaults(GTK_TABLE(ob->table),GTK_WIDGET(ob->filename),1,2,0,1);


	label=gtk_label_new(_("Dimensions:"));
	gtk_misc_set_alignment(GTK_MISC(label),1.0,0.5);
	gtk_table_attach(GTK_TABLE(ob->table),GTK_WIDGET(label),0,1,1,2,
		GtkAttachOptions(GTK_SHRINK|GTK_FILL),GtkAttachOptions(GTK_SHRINK|GTK_FILL),0,0);
	gtk_widget_show(label);

	ob->dimensions=gtk_label_new("");
	gtk_misc_set_alignment(GTK_MISC(ob->dimensions),0.0,0.5);
	gtk_widget_show(ob->dimensions);

	gtk_table_attach_defaults(GTK_TABLE(ob->table),GTK_WIDGET(ob->dimensions),1,2,1,2);


	ob->physicalsize=gtk_label_new("");
	gtk_misc_set_alignment(GTK_MISC(ob->physicalsize),0.0,0.5);
	gtk_widget_show(ob->physicalsize);

	gtk_table_attach_defaults(GTK_TABLE(ob->table),GTK_WIDGET(ob->physicalsize),1,2,2,3);


	label=gtk_label_new(_("Resolution:"));
	gtk_misc_set_alignment(GTK_MISC(label),1.0,0.5);
	gtk_table_attach(GTK_TABLE(ob->table),GTK_WIDGET(label),0,1,3,4,
		GtkAttachOptions(GTK_SHRINK|GTK_FILL),GtkAttachOptions(GTK_SHRINK|GTK_FILL),0,0);
	gtk_widget_show(label);

	ob->resolution=gtk_label_new("");
	gtk_misc_set_alignment(GTK_MISC(ob->resolution),0.0,0.5);
	gtk_widget_show(ob->resolution);

	gtk_table_attach_defaults(GTK_TABLE(ob->table),GTK_WIDGET(ob->resolution),1,2,3,4);


	label=gtk_label_new(_("Profile:"));
	gtk_misc_set_alignment(GTK_MISC(label),1.0,0.5);
	gtk_table_attach(GTK_TABLE(ob->table),GTK_WIDGET(label),0,1,4,5,
		GtkAttachOptions(GTK_SHRINK|GTK_FILL),GtkAttachOptions(GTK_SHRINK|GTK_FILL),0,0);
	gtk_widget_show(label);

	ob->profile=gtk_label_new("");
	gtk_label_set_line_wrap(GTK_LABEL(ob->profile),true);
	gtk_misc_set_alignment(GTK_MISC(ob->profile),0.0,0.5);
	gtk_widget_show(ob->profile);

	gtk_table_attach_defaults(GTK_TABLE(ob->table),GTK_WIDGET(ob->profile),1,2,4,5);

	gtk_widget_show(ob->table);

	pp_imageinfo_change_image(ob);

	return(GTK_WIDGET(ob));
}


GType
pp_imageinfo_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo pp_imageinfo_info =
		{
			sizeof (pp_ImageInfoClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) pp_imageinfo_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (pp_ImageInfo),
			0,
			(GInstanceInitFunc) pp_imageinfo_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_EXPANDER, "pp_ImageInfo", &pp_imageinfo_info, (GTypeFlags)0);
	}
	return stpuic_type;
}


static void
pp_imageinfo_class_init (pp_ImageInfoClass *klass)
{
	pp_imageinfo_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (klass),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (pp_ImageInfoClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
pp_imageinfo_init (pp_ImageInfo *ob)
{
	ob->table=NULL;
	ob->filename=NULL;
	ob->dimensions=NULL;
	ob->profile=NULL;
	ob->scrollwin=NULL;
	ob->thread=NULL;
}


class ii_payload
{
	public:
	ii_payload(const char *fn,pp_ImageInfo *ob,Layout_ImageInfo *ii)
		: filename(NULL), dimensions(NULL), proffilename(NULL), profname(NULL), widget(NULL), imageinfo(NULL)
	{
		filename=strdup(fn);
		widget=ob;
		imageinfo=ii;
		const char *tmp;
		if((tmp=ii->GetAssignedProfile()))
			proffilename=strdup(tmp);
		
	}
	~ii_payload()
	{
		if(filename)
			free(filename);
		if(proffilename)
			free(proffilename);
		if(profname)
			free(profname);
		if(dimensions)
			g_free(dimensions);
	}
	void GetInfo()
	{

	}
	static int ThreadFunc(Thread *t,void *ud)
	{
		t->SendSync();
		ii_payload *p=(ii_payload *)ud;

		ImageSource *is=ISLoadImage(p->filename);
		if(is)
		{
			p->dimensions=g_strdup_printf("%d x %d %s",p->imageinfo->GetWidth(),p->imageinfo->GetHeight(),_("pixels"));

			// Embedded/Assigned Profile
			CMSProfile *prof=NULL;
			if(p->proffilename)
				prof=new CMSProfile(p->proffilename);
			if(!prof)
				prof=is->GetEmbeddedProfile();
			if(prof)
			{
				const char *desc=prof->GetDescription();
				p->profname=strdup(desc);
				if(p->proffilename)	// Only delete the profile if we created it from a filename
					delete prof;
			}
			delete is;
		}
		g_idle_add(ii_payload::IdleFunc,p);
		return(0);
	}
	static gboolean IdleFunc(gpointer ud)
	{
		ii_payload *p=(ii_payload *)ud;
		pp_ImageInfo *ii=p->widget;
		if(p->dimensions)
			gtk_label_set_label(GTK_LABEL(ii->dimensions),p->dimensions);
		if(p->profname)
			gtk_label_set_text(GTK_LABEL(ii->profile),p->profname);
		else
			gtk_label_set_text(GTK_LABEL(ii->profile),_("Default"));
		delete p;
		return(FALSE);
	}
	protected:
	char *filename;
	char *dimensions;
	char *proffilename;
	char *profname;
	pp_ImageInfo *widget;
	Layout_ImageInfo *imageinfo;
};


void pp_imageinfo_change_image(pp_ImageInfo *ob)
{
	if(ob->layout)
	{
		Layout_ImageInfo *ii=ob->layout->FirstSelected();
		if(ii)
		{
			// Remove path leaving just the file part of the filename,
			// and convert to UTF8 for display.
			const char *fn=ii->GetFilename();
			gchar *fnb=g_path_get_basename(fn);
			gsize bread=0,bwritten=0;
			GError *err;
			gchar *fnu=g_filename_to_utf8(fnb,-1,&bread,&bwritten,&err);
			if(fnu)
				gtk_label_set_label(GTK_LABEL(ob->filename),fnu);
			if(fnu)
				g_free(fnu);
			g_free(fnb);

			if(ob->thread)				// Deleting the old thread has the added bonus of ensuring that
				delete ob->thread;		// any previous iteration has completed.

			ii_payload *p=new ii_payload(fn,ob,ii);
			ob->thread=new Thread(ii_payload::ThreadFunc,p);
			ob->thread->Start();
			ob->thread->WaitSync();
	#if 0
			// Dimensions
			ImageSource *is=ISLoadImage(fn);
			if(is)
			{
				gchar *dim=g_strdup_printf("%d x %d %s",is->width,is->height,_("pixels"));
				gtk_label_set_label(GTK_LABEL(ob->dimensions),dim);
				g_free(dim);

				// Embedded/Assigned Profile
				CMSProfile *prof=NULL;
				fn=ii->GetAssignedProfile();
				if(fn)
					prof=new CMSProfile(fn);
				if(!prof)
					prof=is->GetEmbeddedProfile();
				if(prof)
				{
					const char *desc=prof->GetDescription();
					gtk_label_set_text(GTK_LABEL(ob->profile),desc);
					if(fn)	// Only delete the profile if we created it from a filename
						delete prof;
				}
				else
					gtk_label_set_text(GTK_LABEL(ob->profile),"Default");
				delete is;
			}
	#endif
			gtk_widget_set_sensitive(GTK_WIDGET(ob->scrollwin),true);	
		}
	}
	else
	{
		gtk_label_set_text(GTK_LABEL(ob->filename),"");
		gtk_label_set_text(GTK_LABEL(ob->dimensions),"");
		gtk_label_set_text(GTK_LABEL(ob->profile),"");
		gtk_widget_set_sensitive(GTK_WIDGET(ob->scrollwin),false);
	}
}
