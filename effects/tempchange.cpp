#include <iostream>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>

#include "../support/debug.h"

#include "imagesource_util.h"
#include "generaldialogs.h"
#include "tiffsave.h"
#include "progressbar.h"
#include "pathsupport.h"
#include "egg-pixbuf-thumbnail.h"
#include "ppeffect_temperature.h"

char *create_filename(const char *ofn)
{
	char *tmp=strdup(ofn);
	int i=strlen(tmp)-1;
	while(i)
	{
		if(tmp[i]=='.')
		{
			tmp[i]=0;
			break;
		}
		--i;
	}
	char *nfn=(char *)malloc(strlen(tmp)+9);
	Debug[TRACE] << "Building filename from: " << tmp << endl;
	sprintf(nfn,"%s-tc.tif",tmp);
	free(tmp);
	Debug[TRACE] << "Result: " << nfn << endl;
	return(nfn);
}


int main(int argc,char **argv)
{
	try
	{
		gtk_init(&argc,&argv);
		char *filename=NULL;
		if(argc==2)
			filename=strdup(argv[1]);
		else
		{
#ifdef WIN32
			char *dirname=substitute_homedir("$HOME\\My Documents\\My Pictures\\");
#else
			char *dirname=substitute_homedir("$HOME\\");
#endif
			filename=File_Dialog("Open image file...",dirname,NULL,true);
		}

		if(filename)
		{
			GError *err=NULL;
			GdkPixbuf *pb=egg_pixbuf_get_thumbnail_for_file(filename,EGG_PIXBUF_THUMBNAIL_LARGE,&err);
			if(pb)
			{
				PPEffectHeader header;
				PPEffect *effect=new PPEffect_Temperature(header,0,PPEFFECT_PRESCALE);
				if(effect->Dialog(NULL,pb))
				{
					Debug[TRACE] << "Dialog closed with OK button" << endl;
					ImageSource *is=ISLoadImage(filename);
					is=header.ApplyEffects(is,PPEFFECT_PRESCALE);
					char *nfn=create_filename(filename);
					if(nfn)
					{
						char *savefn=File_Save_Dialog("Save TIFF image...",nfn,NULL);
						if(savefn)
						{
							TIFFSaver ts(savefn,is);
							ProgressBar p("Saving image...",false);
							ts.SetProgress(&p);
							ts.Save();
							free(savefn);
						}
						free(nfn);
					}
					delete is;
				}
			}
			free(filename);
		}
	}
	catch(const char *err)
	{
		Debug[TRACE] << "Error: " << err << endl;
	}
	return(0);
}
