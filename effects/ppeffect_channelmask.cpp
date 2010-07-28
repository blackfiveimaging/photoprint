#include <iostream>
#include <stdlib.h>
#include <string.h>

#include <gdk-pixbuf/gdk-pixdata.h>

#include "imagesource_cms.h"
#include "layout.h"

#include "ppeffect_channelmask.h"
//#include "ppeffect_channelmask_icon.cpp"

#include "devicencolorant.h"
#include "imagesource_colorantmask.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)


const char *PPEffect_ChannelMask::ID="ChannelMask";
const char *PPEffect_ChannelMask::GetID()
{
	return(ID);
}

const char *PPEffect_ChannelMask::Name=N_("Select Channels");
const char *PPEffect_ChannelMask::GetName()
{
	return(gettext(Name));
}


PPEffect_ChannelMask::PPEffect_ChannelMask(PPEffectHeader &header,int priority,enum PPEFFECT_STAGE stage)
	:	PPEffect(header,priority,stage), colorants(IS_TYPE_CMYK)
{
}


PPEffect_ChannelMask::~PPEffect_ChannelMask()
{
}


PPEffect_ChannelMask *PPEffect_ChannelMask::Clone(PPEffectHeader &head)
{
	PPEffect_ChannelMask *result=new PPEffect_ChannelMask(head,priority,stage);
	result->SetChannels(GetChannels());
	return(result);
}


ImageSource *PPEffect_ChannelMask::Apply(ImageSource *source)
{
	return(new ImageSource_ColorantMask(source,&colorants));
}


void PPEffect_ChannelMask::SetChannels(std::string channels)
{
	colorants.SetEnabledColorants(channels.c_str());
}


std::string PPEffect_ChannelMask::GetChannels()
{
	char *tmp=colorants.GetEnabledColorants();
	std::string result=tmp;
	free(tmp);
	return (result);
}

