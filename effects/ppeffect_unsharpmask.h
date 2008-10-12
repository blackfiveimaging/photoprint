#ifndef PPEFFECT_UNSHARPMASK_H

#include "ppeffect.h"

class PPEffect_UnsharpMask : public PPEffect
{
	public:
	PPEffect_UnsharpMask(PPEffectHeader &header,int priority,enum PPEFFECT_STAGE stage);
	virtual ~PPEffect_UnsharpMask();
	virtual PPEffect_UnsharpMask *Clone(PPEffectHeader &head);
	virtual ImageSource *Apply(ImageSource *source);
	static const char *ID;
	static const char *Name;
	virtual const char *GetID();
	virtual const char *GetName();
	float GetRadius();
	void SetRadius(float radius);
	float GetAmount();
	void SetAmount(float amount);
	protected:
	float radius;
	float amount;
};

#endif
