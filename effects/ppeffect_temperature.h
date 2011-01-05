#ifndef PPEFFECT_TEMPERATURE_H

#include "ppeffect.h"
#include "lcmswrapper.h"
#include "refcountptr.h"

class PPEffect_Temperature : public PPEffect
{
	public:
	PPEffect_Temperature(PPEffectHeader &header,int priority,enum PPEFFECT_STAGE stage);
	virtual ~PPEffect_Temperature();
	virtual PPEffect_Temperature *Clone(PPEffectHeader &header);
	virtual ImageSource *Apply(ImageSource *source);
//	virtual GtkWidget *SettingsWidget();
//	virtual	bool Dialog(GtkWindow *parent,GdkPixbuf *preview);
	void SetTempChange(int tempchange);
	int GetTempChange();
	static const char *ID;
	static const char *Name;
	virtual const char *GetID();
	virtual const char *GetName();
	protected:
	void MakeTransform(IS_TYPE type);
	int tempchange;
	RefCountPtr<CMSTransform> transform;
};

#endif
