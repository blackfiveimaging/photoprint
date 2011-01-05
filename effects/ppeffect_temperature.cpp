#include <iostream>
#include <stdlib.h>
#include <string.h>

#include <gdk-pixbuf/gdk-pixdata.h>

#include "imagesource_cms.h"
#include "layout.h"

#include "ppeffect_temperature.h"
#include "ppeffect_temperature_icon.cpp"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)


const char *PPEffect_Temperature::ID="Temperature";
const char *PPEffect_Temperature::GetID()
{
	return(ID);
}

const char *PPEffect_Temperature::Name=N_("Warm/Cool");
const char *PPEffect_Temperature::GetName()
{
	return(gettext(Name));
}


PPEffect_Temperature::PPEffect_Temperature(PPEffectHeader &header,int priority,enum PPEFFECT_STAGE stage)
	:	PPEffect(header,priority,stage), tempchange(0)
{
}


PPEffect_Temperature::~PPEffect_Temperature()
{
}


PPEffect_Temperature *PPEffect_Temperature::Clone(PPEffectHeader &head)
{
	PPEffect_Temperature *result=new PPEffect_Temperature(head,priority,stage);
	result->SetTempChange(GetTempChange());
	return(result);
}


void PPEffect_Temperature::MakeTransform(IS_TYPE type)
{
	CMSRGBGamma RGBGamma(2.2,2.2,2.2);
	CMSGamma GreyGamma(2.2);
	CMSWhitePoint srcwp(6500+tempchange);
	CMSWhitePoint dstwp(6500);
	CMSProfile *source;
	switch(STRIP_ALPHA(type))
	{
		case IS_TYPE_RGB:
			source=new CMSProfile(CMSPrimaries_Rec709,RGBGamma,srcwp);
			break;
		case IS_TYPE_GREY:
			source=new CMSProfile(GreyGamma,srcwp);
			break;
		default:
			throw "Only RGB and Greyscale images are currently supported";
	}
	if(!source)
		throw "Can't create source profile";

	CMSProfile dest(CMSPrimaries_Rec709,RGBGamma,dstwp);
	transform=new CMSTransform(source,&dest,LCMSWRAPPER_INTENT_ABSOLUTE_COLORIMETRIC);
	delete source;
}


ImageSource *PPEffect_Temperature::Apply(ImageSource *source)
{
	if(!transform)
		MakeTransform(source->type);

	if(transform->GetInputColourSpace()!=STRIP_ALPHA(source->type))
	{
		transform=NULL;
		MakeTransform(source->type);
	}
	return(new ImageSource_CMS(source,transform));
}


void PPEffect_Temperature::SetTempChange(int tempchange)
{
	transform=NULL;
	this->tempchange=tempchange;
}


int PPEffect_Temperature::GetTempChange()
{
	return (tempchange);
}
