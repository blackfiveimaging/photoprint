#include <iostream>
#include <stdlib.h>
#include <string.h>

#include "imagesource_desaturate.h"
#include "layout.h"

#include "ppeffect_desaturate.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gettext.h"

#define _(x) gettext(x)
#define N_(x) gettext_noop(x)


const char *PPEffect_Desaturate::ID="Desaturate";
const char *PPEffect_Desaturate::GetID()
{
	return(ID);
}
const char *PPEffect_Desaturate::Name=N_("Desaturate");
const char *PPEffect_Desaturate::GetName()
{
	return(Name);
}


PPEffect_Desaturate::PPEffect_Desaturate(PPEffectHeader &header,int priority,enum PPEFFECT_STAGE stage)
	:	PPEffect(header,priority,stage)
{
}


PPEffect_Desaturate::~PPEffect_Desaturate()
{
}


PPEffect_Desaturate *PPEffect_Desaturate::Clone(PPEffectHeader &head)
{
	PPEffect_Desaturate *result=new PPEffect_Desaturate(head,priority,stage);
	return(result);
}


ImageSource *PPEffect_Desaturate::Apply(ImageSource *source)
{
	return(new ImageSource_Desaturate(source));
}

