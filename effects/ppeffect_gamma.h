#ifndef PPEFFECT_GAMMA_H
#define PPEFFECT_GAMMA_H

#include "ppeffect.h"

class CMSTransform;

class PPEffect_Gamma : public PPEffect
{
	public:
	PPEffect_Gamma(PPEffectHeader &header,int priority,enum PPEFFECT_STAGE stage);
	virtual ~PPEffect_Gamma();
	virtual PPEffect_Gamma *Clone(PPEffectHeader &header);
	virtual ImageSource *Apply(ImageSource *source);
	void SetGamma(double gamma);
	double GetGamma();
	static const char *ID;
	static const char *Name;
	virtual const char *GetID();
	virtual const char *GetName();
	protected:
	double gamma;
};

#endif
