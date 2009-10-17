
#include <math.h>
#include <stdio.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkdnd.h>

#include "stpui_widgets/stpui_combo.h"
#include "pp_sigcontrol.h"

#include "support/debug.h"
#include "support/progressbar.h"

#include "pp_menu_image.h"
#include "dialogs.h"

#include "pp_layout_nup_pageview.h"

#define PAGEVIEW_DEFAULT_WIDTH 250
#define PAGEVIEW_DEFAULT_HEIGHT 320


enum {
	CHANGED_SIGNAL,
	REFLOW_SIGNAL,
	SELECTIONCHANGED_SIGNAL,
	POPUPMENU_SIGNAL,
	LAST_SIGNAL
};

static guint pp_layout_nup_pageview_signals[LAST_SIGNAL] = { 0 };

static void pp_layout_nup_pageview_class_init               (pp_Layout_NUp_PageViewClass     *klass);
static void pp_layout_nup_pageview_init                     (pp_Layout_NUp_PageView          *pageview);
static void pp_layout_nup_pageview_realize                  (GtkWidget        *widget);
static void pp_layout_nup_pageview_size_request             (GtkWidget        *widget,
                                               GtkRequisition   *requisition);
static void pp_layout_nup_pageview_size_allocate            (GtkWidget        *widget,
                                               GtkAllocation    *allocation);
static gboolean pp_layout_nup_pageview_expose               (GtkWidget        *widget,
                                               GdkEventExpose   *event);
static gboolean pp_layout_nup_pageview_button_press         (GtkWidget        *widget,
                                               GdkEventButton   *event);
static gboolean pp_layout_nup_pageview_button_release       (GtkWidget        *widget,
                                               GdkEventButton   *event);
static gboolean pp_layout_nup_pageview_motion_notify        (GtkWidget        *widget,
                                               GdkEventMotion   *event );


#define TARGET_URI_LIST 1


static GtkTargetEntry dnd_file_drop_types[] = {
	{ "text/uri-list", 0, TARGET_URI_LIST }
};
static gint dnd_file_drop_types_count = 1;

static void get_dnd_data(GtkWidget *widget, GdkDragContext *context,
				     gint x, gint y,
				     GtkSelectionData *selection_data, guint info,
				     guint time, gpointer data)
{
	gchar *uris=g_strdup((const gchar *)selection_data->data);
	gchar *urilist=uris;
	int lastpage=0;
	pp_Layout_NUp_PageView *pv=PP_LAYOUT_NUP_PAGEVIEW(widget);
	ProgressBar progress("Adding images...",false);
	while(*urilist)
	{
		if(strncmp(urilist,"file:",5))
		{
			while(*urilist && *urilist!='\n' && *urilist!='\r')
				++urilist;
			while(*urilist=='\n' || *urilist=='\r')
				*urilist++;
		}
		else
		{	
			Debug[TRACE] << "URIList: " << urilist << endl;
			gchar *uri=urilist;
			while(*urilist && *urilist!='\n' && *urilist!='\r')
				++urilist;
			if(*urilist)
			{
				while(*urilist=='\n' || *urilist=='\r')
					*urilist++=0;
			}
			if(*uri && *uri!='\n' && *uri!='\r')
			{
				gchar *filename=g_filename_from_uri(uri,NULL,NULL);
				lastpage=pv->layout->AddImage(filename);
				pp_layout_nup_pageview_refresh(pv);
				g_signal_emit_by_name (GTK_OBJECT (pv), "changed");
				progress.DoProgress(0,0);
			}
		}
	}
	pp_layout_nup_pageview_set_page(pv,lastpage);
	g_signal_emit_by_name (GTK_OBJECT (pv), "changed");
	g_free(uris);
}


/* Local data */

static GtkWidgetClass *parent_class = NULL;

GType
pp_layout_nup_pageview_get_type ()
{
  static GType pageview_type = 0;

  if (!pageview_type)
    {
      static const GTypeInfo pageview_info =
      {
	sizeof (pp_Layout_NUp_PageViewClass),
	NULL,
	NULL,
	(GClassInitFunc) pp_layout_nup_pageview_class_init,
	NULL,
	NULL,
	sizeof (pp_Layout_NUp_PageView),
        0,
	(GInstanceInitFunc) pp_layout_nup_pageview_init,
      };

      pageview_type = g_type_register_static (GTK_TYPE_WIDGET, "pp_Layout_NUp_PageView", &pageview_info, GTypeFlags(0));
    }

  return pageview_type;
}


static void
pp_layout_nup_pageview_class_init (pp_Layout_NUp_PageViewClass *cl)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;
	
	object_class = (GtkObjectClass*) cl;
	widget_class = (GtkWidgetClass*) cl;
	
	parent_class = GTK_WIDGET_CLASS(gtk_type_class (gtk_widget_get_type ()));
	
	widget_class->realize = pp_layout_nup_pageview_realize;
	widget_class->expose_event = pp_layout_nup_pageview_expose;
	widget_class->size_request = pp_layout_nup_pageview_size_request;
	widget_class->size_allocate = pp_layout_nup_pageview_size_allocate;
	widget_class->button_press_event = pp_layout_nup_pageview_button_press;
	widget_class->button_release_event = pp_layout_nup_pageview_button_release;
	widget_class->motion_notify_event = pp_layout_nup_pageview_motion_notify;

	pp_layout_nup_pageview_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (cl),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (pp_Layout_NUp_PageViewClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	pp_layout_nup_pageview_signals[REFLOW_SIGNAL] =
	g_signal_new ("reflow",
		G_TYPE_FROM_CLASS (cl),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (pp_Layout_NUp_PageViewClass, reflow),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	pp_layout_nup_pageview_signals[SELECTIONCHANGED_SIGNAL] =
	g_signal_new ("selection_changed",
		G_TYPE_FROM_CLASS (cl),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (pp_Layout_NUp_PageViewClass, selectionchanged),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	pp_layout_nup_pageview_signals[POPUPMENU_SIGNAL] =
	g_signal_new ("popupmenu",
		G_TYPE_FROM_CLASS (cl),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (pp_Layout_NUp_PageViewClass, popupmenu),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
pp_layout_nup_pageview_init (pp_Layout_NUp_PageView *pageview)
{
	pageview->button = 0;
	pageview->timer = 0;
	pageview->dragging=FALSE;
	pageview->selected=NULL;
}


GtkWidget*
pp_layout_nup_pageview_new (Layout_NUp *layout)
{
	pp_Layout_NUp_PageView *pageview;

	pageview = PP_LAYOUT_NUP_PAGEVIEW(g_object_new (pp_layout_nup_pageview_get_type (), NULL));

	gtk_drag_dest_set(GTK_WIDGET(pageview),
			  GtkDestDefaults(GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_DROP),
			  dnd_file_drop_types, dnd_file_drop_types_count,
                          GdkDragAction(GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK));
	g_signal_connect(G_OBJECT(pageview), "drag_data_received",
			 G_CALLBACK(get_dnd_data), NULL);

	pageview->layout=layout;

	return GTK_WIDGET (pageview);
}


static void
pp_layout_nup_pageview_realize (GtkWidget *widget)
{
  pp_Layout_NUp_PageView *pageview;
  GdkWindowAttr attributes;
  gint attributes_mask;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (PP_IS_PAGEVIEW (widget));

  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
  pageview = PP_LAYOUT_NUP_PAGEVIEW (widget);

  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;
  attributes.width = widget->allocation.width;
  attributes.height = widget->allocation.height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.event_mask = gtk_widget_get_events (widget) | 
    GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | 
    GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
    GDK_POINTER_MOTION_HINT_MASK;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.colormap = gtk_widget_get_colormap (widget);

  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
  widget->window = gdk_window_new (widget->parent->window, &attributes, attributes_mask);

  widget->style = gtk_style_attach (widget->style, widget->window);

  gdk_window_set_user_data (widget->window, widget);

  gtk_style_set_background (widget->style, widget->window, GTK_STATE_ACTIVE);
}


static void 
pp_layout_nup_pageview_size_request (GtkWidget      *widget,
		       GtkRequisition *requisition)
{
  requisition->width = PAGEVIEW_DEFAULT_WIDTH;
  requisition->height = PAGEVIEW_DEFAULT_HEIGHT;
}


static void
pp_layout_nup_pageview_size_allocate (GtkWidget     *widget,
			GtkAllocation *allocation)
{
  pp_Layout_NUp_PageView *pageview;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (PP_IS_PAGEVIEW (widget));
  g_return_if_fail (allocation != NULL);

  widget->allocation = *allocation;
  pageview = PP_LAYOUT_NUP_PAGEVIEW (widget);

  if (GTK_WIDGET_REALIZED (widget))
    {

      gdk_window_move_resize (widget->window,
			      allocation->x, allocation->y,
			      allocation->width, allocation->height);

    }
	pageview->layout->FlushHRPreviews();
}


static gboolean
pp_layout_nup_pageview_expose( GtkWidget      *widget,
		 GdkEventExpose *event )
{
	pp_Layout_NUp_PageView *pageview=PP_LAYOUT_NUP_PAGEVIEW (widget);

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (PP_IS_PAGEVIEW (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	if (event->count > 0)
		return FALSE;

	pageview->height=widget->allocation.height;
	pageview->width=(pageview->layout->pagewidth*pageview->height)/pageview->layout->pageheight;
	if(pageview->width>widget->allocation.width)
	{
		pageview->width=widget->allocation.width;
		pageview->height=(pageview->layout->pageheight*pageview->width)/pageview->layout->pagewidth;	
	}
	pageview->top=(widget->allocation.height-pageview->height)/2;
	pageview->left=(widget->allocation.width-pageview->width)/2;

	pageview->layout->GetImageableArea();
	pageview->scale=pageview->width;
	pageview->scale/=pageview->layout->pagewidth;

	pageview->layout->DrawPreview(widget,pageview->left,pageview->top,pageview->width,pageview->height);

	for(int row=0;row<pageview->layout->GetRows();++row)
	{
		for(int column=0;column<pageview->layout->GetColumns();++column)
		{
			LayoutRectangle *s=pageview->layout->GetLayoutRectangle(row,column);
			s->Scale(pageview->scale);

			Layout_ImageInfo *ii=pageview->layout->ImageAt(pageview->layout->GetCurrentPage(),row,column);
			if(ii && ii->GetSelected())
			{
				gdk_draw_rectangle (widget->window,
					widget->style->dark_gc[widget->state],FALSE,
					pageview->left+s->x,pageview->top+s->y,s->w,s->h);
				gdk_draw_rectangle (widget->window,
					widget->style->dark_gc[widget->state],FALSE,
					pageview->left+s->x-1,pageview->top+s->y-1,s->w+2,s->h+2);
			}
			else
				gdk_draw_rectangle (widget->window,
					widget->style->mid_gc[widget->state],FALSE,
					pageview->left+s->x,pageview->top+s->y,s->w,s->h);
			delete s;
		}	
	}

	return FALSE;
}


static gboolean
pp_layout_nup_pageview_button_press( GtkWidget      *widget,
		       GdkEventButton *event )
{
	pp_Layout_NUp_PageView *pageview;
	
	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (PP_IS_PAGEVIEW (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);
	
	pageview = PP_LAYOUT_NUP_PAGEVIEW (widget);
	
	int x=int(event->x-pageview->left);
	int y=int(event->y-pageview->top);
	bool shift=(event->state&GDK_SHIFT_MASK)!=0;
	bool ctrl=(event->state&GDK_CONTROL_MASK)!=0;

	if(x>0 && y>0 && x<pageview->width && y<pageview->height && pageview->scale>0.0)
	{
		double sx=x/pageview->scale;
		double sy=y/pageview->scale;
		Layout_ImageInfo *prevselected=pageview->selected;
		pageview->selected=pageview->layout->ImageAtCoord(int(sx),int(sy));
		switch(event->button)
		{
			case 1:
				if(!(shift||ctrl))
				{
						if(pageview->selected && pageview->selected->allowcropping)
						{
							pageview->dragging=true;
							pageview->init_x=x;
							pageview->init_y=y;
							pageview->init_hpan=pageview->selected->crop_hpan;
							pageview->init_vpan=pageview->selected->crop_vpan;
							gtk_grab_add(widget);
						}
						else
							pageview->dragging=false;
						pageview->layout->SelectNone();
				}
				if(pageview->selected)
				{
					if(ctrl)
						pageview->selected->ToggleSelected();
					else if(shift)
					{
						if(prevselected)
						{
							LayoutIterator it(*pageview->layout);
							Layout_ImageInfo *ii=it.FirstImage();
							bool selecting=false;
							while(ii)
							{
								if((ii==prevselected) || (ii==pageview->selected))
								{
									if(selecting)
										ii->SetSelected(true);
									selecting^=true;
								}
								if(selecting)
									ii->SetSelected(true);
								ii=it.NextImage();
							}
						}
						else
							pageview->selected->SetSelected(true);					
					}
					else
						pageview->selected->SetSelected(true);
				}
				pp_layout_nup_pageview_refresh(pageview);
				g_signal_emit_by_name (GTK_OBJECT (pageview), "selection_changed");
				break;
			case 3:
				if(!(shift||ctrl))
				{
					if(pageview->selected)
					{
						if(!pageview->selected->GetSelected())
							pageview->layout->SelectNone();
					}
					else
						pageview->layout->SelectNone();						
				}
				if(pageview->selected)
				{
					pageview->selected->SetSelected(true);
					pp_layout_nup_pageview_refresh(pageview);
					g_signal_emit_by_name (GTK_OBJECT (pageview), "selection_changed");
					g_signal_emit_by_name (GTK_OBJECT (pageview), "popupmenu");
				}
				break;
			default:
				break;	
		}
	}	
	return FALSE;
}


static gboolean
pp_layout_nup_pageview_button_release( GtkWidget      *widget,
                         GdkEventButton *event )
{
	pp_Layout_NUp_PageView *pageview;
	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (PP_IS_PAGEVIEW (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);
	
	pageview = PP_LAYOUT_NUP_PAGEVIEW (widget);

	switch(event->button)
	{
		case 1:	if (event->button==1)
			pageview->dragging=false;
			gtk_grab_remove(widget);
			break;
		default:
			break;
	}
//	g_signal_emit_by_name (GTK_OBJECT (pageview->adjustment), "value_changed");
	return FALSE;
}


static gboolean
pp_layout_nup_pageview_motion_notify( GtkWidget      *widget,
                        GdkEventMotion *event )
{
	pp_Layout_NUp_PageView *pageview;
	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (PP_IS_PAGEVIEW (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);
	
	pageview = PP_LAYOUT_NUP_PAGEVIEW (widget);

	Layout_ImageInfo *ii=pageview->selected;

	if (pageview->dragging && ii)
    {
		int x,y;
		GdkModifierType mods;
		gdk_window_get_pointer (widget->window, &x, &y, &mods);
			
		int dx=(x-pageview->init_x);
		int dy=(y-pageview->init_y);

		int hp=pageview->init_hpan-dx;
		int vp=pageview->init_vpan-dy;
		if(hp<0) hp=0;
		if(hp>LAYOUT_RECTANGLE_PANNING_MAX) hp=LAYOUT_RECTANGLE_PANNING_MAX;
		if(vp<0) vp=0;
		if(vp>LAYOUT_RECTANGLE_PANNING_MAX) vp=LAYOUT_RECTANGLE_PANNING_MAX;

		ii->crop_hpan=LayoutRectangle_Alignment(hp);
		ii->crop_vpan=LayoutRectangle_Alignment(vp);

		gtk_widget_queue_draw (GTK_WIDGET (pageview));
	}
	return FALSE;
}


void pp_layout_nup_pageview_refresh(pp_Layout_NUp_PageView *pv)
{
	gtk_widget_queue_draw (GTK_WIDGET (pv));
}


void pp_layout_nup_pageview_set_page(pp_Layout_NUp_PageView *pv,int page)
{
	pv->layout->SetCurrentPage(page);
	pp_layout_nup_pageview_refresh(pv);
}
