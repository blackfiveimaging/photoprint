#include <iostream>
#include <string.h>

#include <gdk-pixbuf/gdk-pixdata.h>

#include "effectlist.h"

#include "ppeffect.h"
#include "ppeffect_temperature.h"
#include "effectwidget_tempchange.h"
#include "effectwidget_unsharpmask.h"
#include "ppeffect_temperature_icon.cpp"
#include "ppeffect_desaturate.h"
#include "ppeffect_desaturate_icon.cpp"
#include "ppeffect_unsharpmask.h"
#include "ppeffect_unsharpmask_icon.cpp"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)

using namespace std;

enum EFFECT_PRIORITIES {PRIORITY_UNSHARPMASK,PRIORITY_TEMPERATURE,PRIORITY_DESATURATE};


// Abstract base class

class EffectListEntry
{
	public:
	EffectListEntry()
	{
	}
	virtual ~EffectListEntry()
	{
	}
	virtual const char *GetID()=0;
	virtual const char *GetName()=0;
	virtual GdkPixbuf *GetIcon()=0;
	virtual PPEffect *CreateEffect(PPEffectHeader &header)=0;
	virtual GtkWidget *CreateWidget(PPEffect *effect=NULL)
	{
		return(NULL);
	}
	virtual void EffectToWidget(GtkWidget *widget,PPEffect *effect)
	{
	}
	virtual void WidgetToEffect(GtkWidget *widget,PPEffect *effect)
	{
	}
	protected:
};


// Subclass for Temperature effect


class EffectListEntry_Temperature : public EffectListEntry
{
	public:
	EffectListEntry_Temperature() :
		EffectListEntry(), icon(NULL), tempchange(0)
	{
	}
	virtual ~EffectListEntry_Temperature()
	{
	}
	virtual const char *GetID()
	{
		return(PPEffect_Temperature::ID);
	}
	virtual const char *GetName()
	{
		return(gettext(PPEffect_Temperature::Name));
	}
	virtual GdkPixbuf *GetIcon()
	{
		if(icon)
			return(icon);
		GdkPixdata pd;
		GError *err;

		if(!gdk_pixdata_deserialize(&pd,sizeof(PPEffect_Temperature_Icon),PPEffect_Temperature_Icon,&err))
			throw(err->message);

		if(!(icon=gdk_pixbuf_from_pixdata(&pd,false,&err)))
			throw(err->message);

		return(icon);
	}
	virtual PPEffect *CreateEffect(PPEffectHeader &header)
	{
		PPEffect_Temperature *result=new PPEffect_Temperature(header,PRIORITY_TEMPERATURE,PPEFFECT_PRESCALE);
		result->SetTempChange(tempchange);
		return(result);
	}
	virtual GtkWidget *CreateWidget(PPEffect *effect)
	{
		PPEffect_Temperature *pe=(PPEffect_Temperature *)effect;
		GtkWidget *result=NULL;
		result=effectwidget_tempchange_new();
		if(pe)
			EffectToWidget(result,effect);
		else
			effectwidget_tempchange_set(EFFECTWIDGET_TEMPCHANGE(result),tempchange);
		gtk_widget_set_sensitive(result,(pe!=NULL));
		return(result);
	}
	virtual void EffectToWidget(GtkWidget *widget,PPEffect *effect)
	{
		PPEffect_Temperature *pe=(PPEffect_Temperature *)effect;
		if(pe)
			effectwidget_tempchange_set(EFFECTWIDGET_TEMPCHANGE(widget),pe->GetTempChange());
	}
	virtual void WidgetToEffect(GtkWidget *widget,PPEffect *effect)
	{
		PPEffect_Temperature *pe=(PPEffect_Temperature *)effect;
		tempchange=effectwidget_tempchange_get(EFFECTWIDGET_TEMPCHANGE(widget));
		if(pe)
			pe->SetTempChange(tempchange);
	}
	protected:
	GdkPixbuf *icon;
	int tempchange;
};


// Subclass for Desaturate effect


class EffectListEntry_Desaturate : public EffectListEntry
{
	public:
	EffectListEntry_Desaturate() :
		EffectListEntry(), icon(NULL)
	{
	}
	virtual ~EffectListEntry_Desaturate()
	{
	}
	virtual const char *GetID()
	{
		return(PPEffect_Desaturate::ID);
	}
	virtual const char *GetName()
	{
		return(gettext(PPEffect_Desaturate::Name));
	}
	virtual GdkPixbuf *GetIcon()
	{
		if(icon)
			return(icon);
		GdkPixdata pd;
		GError *err;

		if(!gdk_pixdata_deserialize(&pd,sizeof(PPEffect_Desaturate_Icon),PPEffect_Desaturate_Icon,&err))
			throw(err->message);

		if(!(icon=gdk_pixbuf_from_pixdata(&pd,false,&err)))
			throw(err->message);

		return(icon);
	}
	virtual PPEffect *CreateEffect(PPEffectHeader &header)
	{
		PPEffect_Desaturate *result=new PPEffect_Desaturate(header,PRIORITY_DESATURATE,PPEFFECT_PRESCALE);
		return(result);
	}
	protected:
	GdkPixbuf *icon;
};


// Subclass for UnsharpMask effect


class EffectListEntry_UnsharpMask : public EffectListEntry
{
	public:
	EffectListEntry_UnsharpMask() :
		EffectListEntry(), icon(NULL), radius(3.0), amount(1.5)
	{
	}
	virtual ~EffectListEntry_UnsharpMask()
	{
	}
	virtual const char *GetID()
	{
		return(PPEffect_UnsharpMask::ID);
	}
	virtual const char *GetName()
	{
		return(gettext(PPEffect_UnsharpMask::Name));
	}
	virtual GdkPixbuf *GetIcon()
	{
		if(icon)
			return(icon);
		GdkPixdata pd;
		GError *err;

		if(!gdk_pixdata_deserialize(&pd,sizeof(PPEffect_UnsharpMask_Icon),PPEffect_UnsharpMask_Icon,&err))
			throw(err->message);

		if(!(icon=gdk_pixbuf_from_pixdata(&pd,false,&err)))
			throw(err->message);

		return(icon);
	}
	virtual PPEffect *CreateEffect(PPEffectHeader &header)
	{
		PPEffect_UnsharpMask *result=new PPEffect_UnsharpMask(header,PRIORITY_UNSHARPMASK,PPEFFECT_PRESCALE);
		result->SetRadius(radius);
		result->SetAmount(amount);
		return(result);
	}
	virtual GtkWidget *CreateWidget(PPEffect *effect)
	{
		PPEffect_UnsharpMask *pe=(PPEffect_UnsharpMask *)effect;
		GtkWidget *result=NULL;
		result=effectwidget_unsharpmask_new();
		if(pe)
			EffectToWidget(result,effect);
		else
		{
			effectwidget_unsharpmask_set_radius(EFFECTWIDGET_UNSHARPMASK(result),radius);
			effectwidget_unsharpmask_set_amount(EFFECTWIDGET_UNSHARPMASK(result),amount);
		}
		gtk_widget_set_sensitive(result,(pe!=NULL));
		return(result);
	}
	virtual void EffectToWidget(GtkWidget *widget,PPEffect *effect)
	{
		PPEffect_UnsharpMask *pe=(PPEffect_UnsharpMask *)effect;
		if(pe)
		{
			effectwidget_unsharpmask_set_radius(EFFECTWIDGET_UNSHARPMASK(widget),pe->GetRadius());
			effectwidget_unsharpmask_set_amount(EFFECTWIDGET_UNSHARPMASK(widget),pe->GetAmount());
		}
	}
	virtual void WidgetToEffect(GtkWidget *widget,PPEffect *effect)
	{
		PPEffect_UnsharpMask *pe=(PPEffect_UnsharpMask *)effect;
		radius=effectwidget_unsharpmask_get_radius(EFFECTWIDGET_UNSHARPMASK(widget));
		amount=effectwidget_unsharpmask_get_amount(EFFECTWIDGET_UNSHARPMASK(widget));
		if(pe)
		{
			pe->SetRadius(radius);
			pe->SetAmount(amount);
		}
	}
	protected:
	GdkPixbuf *icon;
	float radius;
	float amount;
};


// Table of effects...

static EffectListEntry_Desaturate effectlistentry_desaturate;
static EffectListEntry_Temperature effectlistentry_temperature;
static EffectListEntry_UnsharpMask effectlistentry_unsharpmask;

static EffectListEntry *effectlistsources[]=
{
	&effectlistentry_desaturate,
	&effectlistentry_temperature,
	&effectlistentry_unsharpmask
};

#define SOURCECOUNT (sizeof(effectlistsources)/sizeof(EffectListEntry *))


// EffectListSource member functions.

EffectListSource::EffectListSource()
{
}


EffectListSource::~EffectListSource()
{
}


int EffectListSource::EffectCount()
{
	return(SOURCECOUNT);
}


const char *EffectListSource::GetID(unsigned int effect)
{
	if((effect>=0) && (effect<SOURCECOUNT))
		return(effectlistsources[effect]->GetID());
	return(NULL);
}


const char *EffectListSource::GetName(unsigned int effect)
{
	if((effect>=0) && (effect<SOURCECOUNT))
		return(effectlistsources[effect]->GetName());
	return(NULL);
}


GdkPixbuf *EffectListSource::GetIcon(unsigned int effect)
{
	if((effect>=0) && (effect<SOURCECOUNT))
		return(effectlistsources[effect]->GetIcon());
	return(NULL);
}


PPEffect *EffectListSource::CreateEffect(unsigned int effect,PPEffectHeader &header)
{
	if((effect>=0) && (effect<SOURCECOUNT))
		return(effectlistsources[effect]->CreateEffect(header));
	return(NULL);
}


GtkWidget *EffectListSource::CreateWidget(unsigned int idx,PPEffect *effect)
{
	if((idx>=0) && (idx<SOURCECOUNT))
		return(effectlistsources[idx]->CreateWidget(effect));
	return(NULL);
}


void EffectListSource::WidgetToEffect(GtkWidget *widget,PPEffect *effect)
{
	const char *id=effect->GetID();
	for(unsigned int i=0;i<SOURCECOUNT;++i)
	{
		if(strcmp(effectlistsources[i]->GetID(),id)==0)
		{
			effectlistsources[i]->WidgetToEffect(widget,effect);
		}
	}
}


void EffectListSource::EffectToWidget(GtkWidget *widget,PPEffect *effect)
{
	const char *id=effect->GetID();
	for(unsigned int i=0;i<SOURCECOUNT;++i)
	{
		if(strcmp(effectlistsources[i]->GetID(),id)==0)
		{
			effectlistsources[i]->EffectToWidget(widget,effect);
		}
	}
}
