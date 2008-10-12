// Class to access a list of available effects.
// Handles both internal and translated names, UI Icons,
// and has a method to create a widget for manipulating the effect.

#ifndef EFFECTLIST_H

#include "ppeffect.h"

class EffectListSource
{
	public:
	EffectListSource();
	virtual ~EffectListSource();
	virtual int EffectCount();
	virtual const char *GetID(unsigned int effect);
	virtual const char *GetName(unsigned int effect);
	virtual GdkPixbuf *GetIcon(unsigned int effect);
	virtual PPEffect *CreateEffect(unsigned int effect,PPEffectHeader &header);
	virtual GtkWidget *CreateWidget(unsigned int effect,PPEffect *peffect=NULL);
	virtual void EffectToWidget(GtkWidget *widget,PPEffect *effect);
	virtual void WidgetToEffect(GtkWidget *widget,PPEffect *effect);
	private:
};

#endif
