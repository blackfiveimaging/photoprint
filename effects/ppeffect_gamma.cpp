#include <iostream>
#include <stdlib.h>
#include <string.h>

#include <gdk-pixbuf/gdk-pixdata.h>

#include "imagesource_cms.h"
#include "layout.h"

#include "ppeffect_gamma.h"
#include "ppeffect_gamma_icon.cpp"

#include "imagesource_gamma.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)


const char *PPEffect_Gamma::ID="Gamma";
const char *PPEffect_Gamma::GetID()
{
	return(ID);
}

const char *PPEffect_Gamma::Name=N_("Gamma adjust");
const char *PPEffect_Gamma::GetName()
{
	return(gettext(Name));
}


PPEffect_Gamma::PPEffect_Gamma(PPEffectHeader &header,int priority,enum PPEFFECT_STAGE stage)
	:	PPEffect(header,priority,stage), gamma(1.0)
{
}


PPEffect_Gamma::~PPEffect_Gamma()
{
}


PPEffect_Gamma *PPEffect_Gamma::Clone(PPEffectHeader &head)
{
	PPEffect_Gamma *result=new PPEffect_Gamma(head,priority,stage);
	result->SetGamma(GetGamma());
	return(result);
}


ImageSource *PPEffect_Gamma::Apply(ImageSource *source)
{
	return(new ImageSource_Gamma(source,1.0/gamma));
}


void PPEffect_Gamma::SetGamma(double gamma)
{
	this->gamma=gamma;
}


double PPEffect_Gamma::GetGamma()
{
	return (gamma);
}
