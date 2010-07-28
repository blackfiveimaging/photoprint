#ifndef PPEFFECT_CHANNELMASK_H
#define PPEFFECT_CHANNELMASK_H

#include <string>
#include "ppeffect.h"

#include "devicencolorant.h"

class CMSTransform;

class EffectListEntry_ChannelMask;
class PPEffect_ChannelMask : public PPEffect
{
	public:
	PPEffect_ChannelMask(PPEffectHeader &header,int priority,enum PPEFFECT_STAGE stage);
	virtual ~PPEffect_ChannelMask();
	virtual PPEffect_ChannelMask *Clone(PPEffectHeader &header);
	virtual ImageSource *Apply(ImageSource *source);
	void SetChannels(std::string channels);
	std::string GetChannels();
	static const char *ID;
	static const char *Name;
	virtual const char *GetID();
	virtual const char *GetName();
	protected:
	DeviceNColorantList colorants;
	friend class EffectListEntry_ChannelMask;
};

#endif
