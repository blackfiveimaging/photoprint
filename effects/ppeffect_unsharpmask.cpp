#include <iostream>
#include <stdlib.h>
#include <string.h>

#include "imagesource_unsharpmask.h"
#include "layout.h"

#include "ppeffect_unsharpmask.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gettext.h"

#define _(x) gettext(x)
#define N_(x) gettext_noop(x)


const char *PPEffect_UnsharpMask::ID="UnsharpMask";
const char *PPEffect_UnsharpMask::GetID()
{
	return(ID);
}
const char *PPEffect_UnsharpMask::Name=_("Sharpen");
const char *PPEffect_UnsharpMask::GetName()
{
	return(Name);
}


PPEffect_UnsharpMask::PPEffect_UnsharpMask(PPEffectHeader &header,int priority,enum PPEFFECT_STAGE stage)
	:	PPEffect(header,priority,stage), radius(3.0), amount(1.5)
{
}


PPEffect_UnsharpMask::~PPEffect_UnsharpMask()
{
}


PPEffect_UnsharpMask *PPEffect_UnsharpMask::Clone(PPEffectHeader &head)
{
	PPEffect_UnsharpMask *result=new PPEffect_UnsharpMask(head,priority,stage);
	result->SetRadius(GetRadius());
	result->SetAmount(GetAmount());
	return(result);
}


ImageSource *PPEffect_UnsharpMask::Apply(ImageSource *source)
{
	return(new ImageSource_UnsharpMask(source,radius,amount));
}


float PPEffect_UnsharpMask::GetRadius()
{
	return(radius);
}


void PPEffect_UnsharpMask::SetRadius(float radius)
{
	this->radius=radius;
}


float PPEffect_UnsharpMask::GetAmount()
{
	return(amount);
}


void PPEffect_UnsharpMask::SetAmount(float amount)
{
	this->amount=amount;
}
