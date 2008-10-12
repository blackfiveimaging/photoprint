#ifndef PPEFFECT_DESATURATE_H

#include "ppeffect.h"

class PPEffect_Desaturate : public PPEffect
{
	public:
	PPEffect_Desaturate(PPEffectHeader &header,int priority,enum PPEFFECT_STAGE stage);
	virtual ~PPEffect_Desaturate();
	virtual PPEffect_Desaturate *Clone(PPEffectHeader &header);
	virtual ImageSource *Apply(ImageSource *source);
//	virtual	bool Dialog(GtkWindow *parent,GdkPixbuf *preview);
//	virtual GtkWidget *SettingsWidget();
	static const char *ID;
	static const char *Name;
	virtual const char *GetID();
	virtual const char *GetName();
	protected:
};

#endif
