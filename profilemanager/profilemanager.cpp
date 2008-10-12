#include <iostream>

#ifdef WIN32
#include <windows.h>
#else
#include <X11/Xatom.h>
#endif

#include "profilemanager.h"
#include "searchpathdbhandler.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gettext.h"
#define _(x) gettext(x)

#define SYSTEMMONITORPROFILE_ESCAPESTRING "<System monitor profile>"
#define BUILTINSRGB_ESCAPESTRING "<Built-in sRGB profile>"
using namespace std;


// ProfileManager


ConfigTemplate ProfileManager::Template[]=
{
 	ConfigTemplate("DefaultRGBProfile","BUILTINSRGB_ESCAPESTRING>"),
	ConfigTemplate("DefaultRGBProfileActive",int(1)),
	ConfigTemplate("DefaultCMYKProfile","USWebCoatedSWOP.icc"),
	ConfigTemplate("DefaultCMYKProfileActive",int(1)),
	ConfigTemplate("DefaultGreyProfile",""),
	ConfigTemplate("DefaultGreyProfileActive",int(0)),
	ConfigTemplate("PrinterProfile",""),
	ConfigTemplate("PrinterProfileActive",int(0)),
	ConfigTemplate("ExportProfile",""),
	ConfigTemplate("ExportProfileActive",int(0)),
	ConfigTemplate("MonitorProfile",SYSTEMMONITORPROFILE_ESCAPESTRING),
	ConfigTemplate("MonitorProfileActive",int(1)),
	ConfigTemplate("RenderingIntent",int(LCMSWRAPPER_INTENT_PERCEPTUAL)),
	ConfigTemplate("ProofMode",int(CM_PROOFMODE_NONE)),
#ifdef WIN32
	ConfigTemplate("ProfilePath","c:\\winnt\\system32\\spool\\drivers\\color\\;c:\\windows\\system32\\spool\\drivers\\color"),
#else
	ConfigTemplate("ProfilePath","/usr/share/color/icc:/usr/local/share/color/icc/:$HOME/.color/icc"),
#endif
	ConfigTemplate()
};


ProfileManager::ProfileManager(ConfigFile *inifile,const char *section) :
	ConfigDB(Template), SearchPathHandler(), first(NULL), proffromdisplay_size(0)
{
#ifndef WIN32
	xdisplay = XOpenDisplay(NULL);
 	proffromdisplay=NULL;
#endif
	new SearchPathHandlerDBHandler(inifile,section,this,this,"ProfilePath");
	AddPath(FindString("ProfilePath"));
	GetProfileFromDisplay();
	if(proffromdisplay_size)
		SetInt("MonitorProfileActive",1);
}


ProfileManager::~ProfileManager()
{
#ifndef WIN32
	if(proffromdisplay)
		XFree(proffromdisplay);
	proffromdisplay=NULL;
	XCloseDisplay (xdisplay);
#endif
	ProfileInfo *pi;
	while((pi=first))
		delete first;
}


CMSProfile *ProfileManager::GetProfile(const char *name)
{
	CMSProfile *result=NULL;
	if(name && strlen(name))
	{
		if(strcmp(name,SYSTEMMONITORPROFILE_ESCAPESTRING)==0 && proffromdisplay_size)
		{
#ifdef WIN32	
			result=new CMSProfile(displayprofilename);
#else
			result=new CMSProfile((char *)proffromdisplay,proffromdisplay_size);
#endif
			if(!result)
			{
				cerr << "Couldn't open monitor profile - falling back to builtin sRGB" << endl;
				result=new CMSProfile();
			}
		}
		else if(strcmp(name,BUILTINSRGB_ESCAPESTRING)==0)
		{
			result=new CMSProfile();
		}
		else
		{
			char *fn=Search(name);
			if(fn && strlen(fn))
			{
				try
				{
					result=new CMSProfile(fn);
				}
				catch(const char *err)
				{
					free(fn);
					throw err;
				}
			}
			if(fn)
				free(fn);
		}
	}
	return(result);
}


CMSProfile *ProfileManager::GetProfile(enum CMColourDevice target)
{
	const char *profilename=NULL;
	switch(target)
	{
		case CM_COLOURDEVICE_DISPLAY:
			if(FindInt("MonitorProfileActive"))
				profilename=FindString("MonitorProfile");
			break;
		case CM_COLOURDEVICE_PRINTERPROOF:
			if(FindInt("MonitorProfileActive"))
				profilename=FindString("MonitorProfile");
			break;
		case CM_COLOURDEVICE_EXPORT:
			if(FindInt("ExportProfileActive"))
				profilename=FindString("ExportProfile");
			break;
		case CM_COLOURDEVICE_PRINTER:
			if(FindInt("PrinterProfileActive"))
				profilename=FindString("PrinterProfile");
			break;
		case CM_COLOURDEVICE_DEFAULTGREY:
			if(FindInt("DefaultGreyProfileActive"))
				profilename=FindString("DefaultGreyProfile");
			break;
		case CM_COLOURDEVICE_DEFAULTRGB:
			if(FindInt("DefaultRGBProfileActive"))
				profilename=FindString("DefaultRGBProfile");
			break;
		case CM_COLOURDEVICE_DEFAULTCMYK:
			if(FindInt("DefaultCMYKProfileActive"))
				profilename=FindString("DefaultCMYKProfile");
			break;
		default:
			break;
	}
	if(profilename)
		return(GetProfile(profilename));
	else
		return(NULL);
}


CMSProfile *ProfileManager::GetDefaultProfile(IS_TYPE colourspace)
{
	const char *profilename=NULL;
	switch(STRIP_ALPHA(colourspace))
	{
		case IS_TYPE_GREY:
			if(FindInt("DefaultGreyProfileActive"))
				profilename=FindString("DefaultGreyProfile");
			break;
		case IS_TYPE_RGB:
			if(FindInt("DefaultRGBProfileActive"))
				profilename=FindString("DefaultRGBProfile");
			break;
		case IS_TYPE_CMYK:
			if(FindInt("DefaultCMYKProfileActive"))
				profilename=FindString("DefaultCMYKProfile");
			break;
		default:
			throw "Image colourspace not yet handled...";
			break;
	}
	if(profilename)
		return(GetProfile(profilename));
	else
		return(NULL);
}


CMTransformFactory *ProfileManager::GetTransformFactory()
{
	return(new CMTransformFactory(*this));
}


// CMTransformFactory

// CMTransformFactoryNode

CMTransformFactoryNode::CMTransformFactoryNode(CMTransformFactory *header,CMSTransform *transform,MD5Digest &d1,MD5Digest &d2,LCMSWrapper_Intent intent,bool proof)
	: header(header), prev(NULL), next(NULL), transform(transform), digest1(d1), digest2(d2), intent(intent), proof(proof)
{
	prev=header->first;
	if((prev=header->first))
	{
		while(prev->next)
			prev=prev->next;
		prev->next=this;
	}
	else
		header->first=this;
}


CMTransformFactoryNode::~CMTransformFactoryNode()
{
	if(next)
		next->prev=prev;
	if(prev)
		prev->next=next;
	else
		header->first=next;

	if(transform)
		delete transform;
}


// CMTransformFactory proper


CMTransformFactory::CMTransformFactory(ProfileManager &pm)
	: manager(pm), first(NULL)
{
}


CMTransformFactory::~CMTransformFactory()
{
	while(first)
		delete first;
}


CMSTransform *CMTransformFactory::Search(MD5Digest *srcdigest,MD5Digest *dstdigest,LCMSWrapper_Intent intent,bool proof)
{
	CMTransformFactoryNode *tfn=first;
	while(tfn)
	{
		if((*srcdigest==tfn->digest1)&&(*dstdigest==tfn->digest2)&&(intent==tfn->intent)&&(proof==tfn->proof))
			return(tfn->transform);
		tfn=tfn->next;
	}
	return(NULL);
}


CMSTransform *CMTransformFactory::GetTransform(enum CMColourDevice target,IS_TYPE type,LCMSWrapper_Intent intent)
{
	CMSProfile *srcprofile=manager.GetDefaultProfile(type);
	CMSTransform *t=NULL;
	try
	{
		t=GetTransform(target,srcprofile,intent);
	}
	catch(const char *err)
	{
	}
	delete srcprofile;
	return(t);
}


CMSTransform *CMTransformFactory::GetTransform(enum CMColourDevice target,CMSProfile *srcprofile,LCMSWrapper_Intent intent)
{
	CMSTransform *result=NULL;
	// If a NULL profile is supplied, we currently bail out.
	// Theoretically we could continue if the target's profile is a DeviceLink,
	// or we could assume a colourspace to match the target's profile,
	// and fall back gracefully.

	if(!srcprofile)
		return(NULL);

	CMSProfile *destprofile=manager.GetProfile(target);

	if(target==CM_COLOURDEVICE_PRINTERPROOF)
	{
		CMSProfile *proofprofile=NULL;
		switch(manager.FindInt("ProofMode"))
		{
			case CM_PROOFMODE_NONE:
//				cerr << "Proofmode: None - using normal transform" << endl;
				result=GetTransform(destprofile,srcprofile,intent);
				break;
			case CM_PROOFMODE_SIMULATEPRINT:
//				cerr << "Proofmode: Simulate Printer - using abs.col. proof transform" << endl;
				proofprofile=manager.GetProfile(CM_COLOURDEVICE_PRINTER);
				if(proofprofile)
					result=GetTransform(destprofile,srcprofile,proofprofile,intent,LCMSWRAPPER_INTENT_ABSOLUTE_COLORIMETRIC);
				break;
			case CM_PROOFMODE_SIMULATEPRINTADAPTWHITE:
//				cerr << "Proofmode: Simulate Printer, Adapt White - using rel.col. proof transform" << endl;
				proofprofile=manager.GetProfile(CM_COLOURDEVICE_PRINTER);
				if(proofprofile)
					result=GetTransform(destprofile,srcprofile,proofprofile,intent,LCMSWRAPPER_INTENT_RELATIVE_COLORIMETRIC);
				break;
			default:
				break;
		}
		if(proofprofile)
			delete proofprofile;
	}
	else
		result=GetTransform(destprofile,srcprofile,intent);

	delete destprofile;

	return(result);
}


CMSTransform *CMTransformFactory::GetTransform(CMSProfile *destprofile,CMSProfile *srcprofile,LCMSWrapper_Intent intent)
{
	// No point whatever in continuing without an output device profile...
	if(!destprofile)
		return(NULL);

	CMSTransform *transform=NULL;
	if(intent==LCMSWRAPPER_INTENT_DEFAULT)
		intent=LCMSWrapper_Intent(manager.FindInt("RenderingIntent"));
	if(intent==LCMSWRAPPER_INTENT_DEFAULT)
		intent=LCMSWRAPPER_INTENT_PERCEPTUAL;

//	cerr << "Using intent: " << intent << endl;

	// We use MD5 digests to compare profiles for equality.
	MD5Digest *d1,*d2;
	d2=destprofile->GetMD5();

	if(destprofile->IsDeviceLink())
	{
//		cerr << "Device link profile detected" << endl;
		// Device link profiles make life awkward if we have to use a source profile
		// (which we must do in the case of an image having an embedded profile).
		// What we do here is convert from the source profile to the appropriate
		// colour space's default profile, and then apply the devicelink.

		CMSProfile *defprofile=NULL;
		if(srcprofile)
			defprofile=manager.GetDefaultProfile(srcprofile->GetColourSpace());

		// If we have both source and default profiles, and they're not equal,
		// create a multi-profile transform: src -> default -> devicelink.
		if((srcprofile)&&(defprofile)&&(*srcprofile->GetMD5()!=*defprofile->GetMD5()))
		{
//			cerr << "Source and default profiles don't match - building transform chain..." << endl;
			CMSProfile *profiles[3];
			profiles[0]=srcprofile;
			profiles[1]=defprofile;
			profiles[2]=destprofile;
			d1=srcprofile->GetMD5();
			
			// Search for an existing transform by source / devicelink MD5s...
			transform=Search(d1,d2,intent);
			if(!transform)
			{
//				cerr << "No suitable cached transform found - creating a new one..." << endl;
				transform=new CMSTransform(profiles,3,intent);
				new CMTransformFactoryNode(this,transform,*d1,*d2,intent);
			}
		}
		else
		{
//			cerr << "Source and default profiles match - using devicelink in isolation..." << endl;
			// If there's no default profile, or the source and default profiles match
			// then we can just use the devicelink profile in isolation.
			d1=d2;
			transform=Search(d1,d2,intent);
			if(!transform)
			{
//				cerr << "No suitable cached transform found - creating a new one..." << endl;
				transform=new CMSTransform(destprofile,intent);
				new CMTransformFactoryNode(this,transform,*d1,*d2,intent);
			}
		}
		if(defprofile)
			delete defprofile;
	}
	else
	{
		// The non-device link case is much easier to deal with...
		d1=srcprofile->GetMD5();

		// Don't bother transforming if src/dest are the same profile...
		if(*d1==*d2)
		{
			cerr << "Source and target profiles are identical - no need to transform" << endl;
			return(NULL);
		}

		transform=Search(d1,d2,intent);
		if(!transform)
		{
//			cerr << "No suitable cached transform found - creating a new one..." << endl;
			transform=new CMSTransform(srcprofile,destprofile,intent);
			new CMTransformFactoryNode(this,transform,*d1,*d2,intent);
		}
	}

	return(transform);
}

CMSTransform *CMTransformFactory::GetTransform(CMSProfile *destprofile,CMSProfile *srcprofile,CMSProfile *proofprofile,LCMSWrapper_Intent intent,int displayintent)
{
//	cerr << "Getting proofing transform - Using intent: " << intent << endl;

	// No point whatever in continuing without an output device profile...
	if(!destprofile)
		return(NULL);

	if(!proofprofile)
		throw _("No Proof profile provided!");

	CMSTransform *transform=NULL;
	if(intent==LCMSWRAPPER_INTENT_DEFAULT)
		intent=LCMSWrapper_Intent(manager.FindInt("RenderingIntent"));
	if(intent==LCMSWRAPPER_INTENT_DEFAULT)
		intent=LCMSWRAPPER_INTENT_PERCEPTUAL;

	if(displayintent==LCMSWRAPPER_INTENT_DEFAULT)
		displayintent=LCMSWRAPPER_INTENT_ABSOLUTE_COLORIMETRIC;

//	cerr << "Using intent: " << intent << endl;

	// We use MD5 digests to compare profiles for equality.
	MD5Digest *d1,*d2;
	d2=destprofile->GetMD5();

	if(destprofile->IsDeviceLink())
	{
//		cerr << "Device link profile detected" << endl;
		// Device link profiles make life awkward if we have to use a source profile
		// (which we must do in the case of an image having an embedded profile).
		// What we do here is convert from the source profile to the appropriate
		// colour space's default profile, and then apply the devicelink.

		CMSProfile *defprofile=NULL;
		if(srcprofile)
			defprofile=manager.GetDefaultProfile(srcprofile->GetColourSpace());

		// If we have both source and default profiles, and they're not equal,
		// create a multi-profile transform: src -> default -> devicelink.
		if((srcprofile)&&(defprofile)&&(*srcprofile->GetMD5()!=*defprofile->GetMD5()))
		{
//			cerr << "Source and default profiles don't match - building transform chain..." << endl;
			CMSProfile *profiles[3];
			profiles[0]=srcprofile;
			profiles[1]=defprofile;
			profiles[2]=destprofile;
			d1=srcprofile->GetMD5();
			
			// Search for an existing transform by source / devicelink MD5s...
			transform=Search(d1,d2,intent,true);
			if(!transform)
			{
//				cerr << "No suitable cached transform found - creating a new one..." << endl;
//				cerr << "But can't (yet?) create embedded->default->devicelink->proof transform!" << endl;
				// FIXME - need a version of CMSProofingTransform that can cope with
				// multiple profiles!
				transform=new CMSTransform(profiles,3,intent);
				new CMTransformFactoryNode(this,transform,*d1,*d2,intent);
			}
		}
		else
		{
//			cerr << "Source and default profiles match - using devicelink in isolation..." << endl;
			// If there's no default profile, or the source and default profiles match
			// then we can just use the devicelink profile in isolation.
			d1=d2;
			transform=Search(d1,d2,intent,true);
			if(!transform)
			{
//				cerr << "No suitable cached transform found - creating a new one..." << endl;
				// FIXME - need a version of CMSProofingTransform that can cope with
				// devicelink profiles
				transform=new CMSProofingTransform(destprofile,proofprofile,intent,displayintent);
				new CMTransformFactoryNode(this,transform,*d1,*d2,intent,true);
			}
		}
		if(defprofile)
			delete defprofile;
	}
	else
	{
		// The non-device link case is much easier to deal with...
		d1=srcprofile->GetMD5();

		// Don't bother transforming if src/dest are the same profile...
		// (Actually, if we're proofing, we still need to transform, after all!
//		if(*d1==*d2)
//			return(NULL);

		transform=Search(d1,d2,intent,true);
		if(!transform)
		{
//			cerr << "No suitable cached transform found - creating a new proofing transform..." << endl;
			transform=new CMSProofingTransform(srcprofile,destprofile,proofprofile,intent,displayintent);
			new CMTransformFactoryNode(this,transform,*d1,*d2,intent,true);
		}
	}

	return(transform);
}


void CMTransformFactory::Flush()
{
	while(first)
		delete first;
}


// Path handling

static const char *findextension(const char *filename)
{
	int t=strlen(filename)-1;
	int c;
	for(c=t;c>0;--c)
	{
		if(filename[c]=='.')
			return(filename+c);
	}
	return(filename);
}


const char *ProfileManager::GetNextFilename(const char *prev)
{
	const char *result=prev;
	while((result=SearchPathHandler::GetNextFilename(result)))
	{
		const char *ext=findextension(result);
		if(strncasecmp(ext,".ICM",4)==0)
			return(result);
		if(strncasecmp(ext,".icm",4)==0)
			return(result);
		if(strncasecmp(ext,".ICC",4)==0)
			return(result);
		if(strncasecmp(ext,".icc",4)==0)
			return(result);
	}
	return(result);
}


void ProfileManager::AddPath(const char *path)
{
	FlushProfileInfoList();
	SearchPathHandler::AddPath(path);
}


void ProfileManager::RemovePath(const char *path)
{
	FlushProfileInfoList();
	SearchPathHandler::RemovePath(path);
}


void ProfileManager::ClearPaths()
{
	FlushProfileInfoList();
	SearchPathHandler::ClearPaths();
}


ProfileInfo *ProfileManager::GetFirstProfileInfo()
{
	if(!first)
		BuildProfileInfoList();
	return(first);
}


void ProfileManager::BuildProfileInfoList()
{
//	cerr << "Building ProfileInfo List:" << endl;
	const char *f=NULL;
	FlushProfileInfoList();
	new ProfileInfo(*this,BUILTINSRGB_ESCAPESTRING);
	while((f=GetNextFilename(f)))
	{
		if(!(FindProfileInfo(f)))
			new ProfileInfo(*this,f);
	}
	GetProfileFromDisplay();
	if(proffromdisplay_size)
		new ProfileInfo(*this,SYSTEMMONITORPROFILE_ESCAPESTRING);
}


void ProfileManager::FlushProfileInfoList()
{
	while(first)
		delete first;
}


ProfileInfo *ProfileManager::FindProfileInfo(const char *filename)
{
	ProfileInfo *pi=first;
	while(pi)
	{
		const char *fn=pi->filename;
		if(strcmp(fn,filename)==0)
		{
			cerr << "Found " << filename << endl;
			return(pi);
		}
		pi=pi->Next();
	}
	return(NULL);
}


int ProfileManager::GetProfileInfoCount()
{
	int c=0;
	ProfileInfo *pi=GetFirstProfileInfo();
	while(pi)
	{
		++c;
		pi=pi->Next();
	}
	return(c);
}


ProfileInfo *ProfileManager::GetProfileInfo(int i)
{
	ProfileInfo *pi=GetFirstProfileInfo();
	while(i && pi)
	{
		--i;
		pi=pi->Next();
	}
	return(pi);
}


// ProfileInfo


ProfileInfo::ProfileInfo(ProfileManager &pm,const char *filename)
	: profilemanager(pm), next(NULL), prev(NULL), filename(NULL), iscached(false), description(NULL), isdevicelink(false)
{
	if((next=profilemanager.first))
		next->prev=this;
	profilemanager.first=this;
	this->filename=strdup(filename);
}


ProfileInfo::~ProfileInfo()
{
	if(filename)
		free(filename);
	if(description)
		free(description);
	if(prev)
		prev->next=next;
	else
		profilemanager.first=next;
	if(next)
		next->prev=prev;
}


ProfileInfo *ProfileInfo::Next()
{
	return(next);
}


void ProfileInfo::GetInfo()
{
	if(iscached)
		return;

	CMSProfile *profile=profilemanager.GetProfile(filename);
	if(profile)
	{
		colourspace=profile->GetColourSpace();
		isdevicelink=profile->IsDeviceLink();
		description=strdup(profile->GetDescription());
		delete profile;
		iscached=true;
	}
}


const char *ProfileInfo::GetFilename()
{
	GetInfo();
	return(filename);
}


const char *ProfileInfo::GetDescription()
{
	GetInfo();
	return(description);
}


bool ProfileInfo::IsDeviceLink()
{
	GetInfo();
	return(isdevicelink);
}


IS_TYPE ProfileInfo::GetColourSpace()
{
	GetInfo();
	return(colourspace);
}


int ProfileManager::GetIntentCount()
{
	return(CMS_GetIntentCount());
}

const char *ProfileManager::GetIntentName(LCMSWrapper_Intent intent)
{
	if(intent==LCMSWRAPPER_INTENT_DEFAULT)
		return("Default");
	else
		return(CMS_GetIntentName(intent));
}

const char *ProfileManager::GetIntentDescription(LCMSWrapper_Intent intent)
{
	if(intent==LCMSWRAPPER_INTENT_DEFAULT)
		return("");
	else
		return(CMS_GetIntentDescription(intent));
}


void ProfileManager::GetProfileFromDisplay()
{
#ifdef WIN32
	HDC handle=GetDC(0);	// Get the default screen handle
	DWORD dpsize=sizeof(displayprofilename)-1;

	// Older Mingw wingdi.h files have a faulty definition of the following function,
	// with the second param as DWORD instead of LPDWORD.
	if(GetICMProfile(handle,&dpsize,displayprofilename))
	{
		proffromdisplay_size=dpsize;
		cerr << "Got profile: " << displayprofilename << ", " << displayprofilename << " characters" << endl;
	}
	else
		cerr << "No profile associated with default display." << endl;
#else
	if(proffromdisplay)
		XFree(proffromdisplay);
	proffromdisplay=NULL;

	if(xdisplay)
	{
//		cerr << "Got display" << endl;
		Atom icc_atom;
		icc_atom = XInternAtom (xdisplay, "_ICC_PROFILE", False);
		if (icc_atom != None)
		{
//			cerr << "Got atom" << endl;
			Window w=DefaultRootWindow(xdisplay);
			if(w)
			{
//				cerr << "Got window" << endl;
				Atom type;
				int format=0;
				unsigned long nitems=0;
				unsigned long bytes_after=0;
				int result=0;

				result = XGetWindowProperty (xdisplay, w, icc_atom, 0,
					0x7fffffff,0, XA_CARDINAL,
					&type, &format, &nitems,
					&bytes_after, &proffromdisplay);
				proffromdisplay_size=nitems*(format/8);

				if(result!=Success)
				{
					cerr <<"Failed to retrieve ICC Profile from display..." << endl;
					proffromdisplay=NULL;
					proffromdisplay_size=0;
				}
			}
		}
	}
//	XCloseDisplay (dpy);
#endif
}
