#include <iostream>
#include <stdlib.h>
#include <string.h>

#include "imagesource_util.h"
#include "imagesource_dither.h"
#include "imagesource_solid.h"
#include "imagesource_cms.h"
#include "imagesource_desaturate.h"
#include "imagesource_rotate.h"
#include "imagesource_bilinear.h"
#include "imagesource_lanczossinc.h"
#include "imagesource_downsample.h"
#include "imagesource_convolution.h"
#include "imagesource_gaussianblur.h"
#include "imagesource_unsharpmask.h"
#include "convkernel_gaussian.h"
#include "convkernel_unsharpmask.h"
#include "tiffsave.h"
#include "progresstext.h"


int main(int argc,char **argv)
{
	try
	{
#if 0
		if(argc==4)
		{
			int tempchange=atoi(argv[2]);
			ImageSource *is=ISLoadImage(argv[1]);

			CMSRGBGamma RGBGamma(2.2,2.2,2.2);
			CMSGamma	GreyGamma(2.2);
			CMSWhitePoint srcwp(6500+tempchange);
			CMSWhitePoint dstwp(6500);
			CMSProfile *source;
			switch(STRIP_ALPHA(is->type))
			{
				case IS_TYPE_RGB:
					is=new ImageSource_Desaturate(is);
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
			CMSTransform transform(source,&dest,LCMSWRAPPER_INTENT_ABSOLUTE_COLORIMETRIC);
			is=new ImageSource_CMS(is,&transform);
			delete source;
			TIFFSaver ts(argv[3],is);
			ProgressText p;
			ts.SetProgress(&p);
			ts.Save();
			delete is;
		}
#endif
#if 0
		if(argc==5)
		{
			ImageSource *is=ISLoadImage(argv[1]);
			int w=atoi(argv[2]);
			int h=atoi(argv[3]);
			is=new ImageSource_Downsample(is,w,h);
			TIFFSaver ts(argv[4],is);
			ProgressText p;
			ts.SetProgress(&p);
			ts.Save();
			delete is;
		}
		if(argc==4)
		{
			double pct=atof(argv[2]);
			ImageSource *is=ISLoadImage(argv[1]);
			cerr << "Original size: " << is->width << " by " << is->height << endl;
			is=new ImageSource_Downsample(is,int((is->width*pct)/100.0),int((is->height*pct)/100.0));
			TIFFSaver ts(argv[3],is);
			ProgressText p;
			ts.SetProgress(&p);
			ts.Save();
			delete is;
		}

		if(argc==6)
		{
			double radius=atof(argv[2]);
			double amount=atof(argv[3]);
			double threshold=atof(argv[4]);
			ImageSource *is=ISLoadImage(argv[1]);
//			ConvKernel_Gaussian kernel(radius);
//			ConvKernel_UnsharpMask kernel(radius,amount);
//			kernel.Normalize();
//			is=new ImageSource_Convolution(is,&kernel);
			is=new ImageSource_UnsharpMask(is,radius,amount,threshold);
			TIFFSaver ts(argv[5],is);
			ProgressText p;
			ts.SetProgress(&p);
			ts.Save();
			delete is;
		}
#endif
		if(argc==3)
		{
			ImageSource *is=ISLoadImage(argv[1]);
			is=new ImageSource_GaussianBlur(is,5);
			TIFFSaver ts(argv[2],is,false);
			ProgressText p;
			ts.SetProgress(&p);
			ts.Save();
			delete is;	
		}
		if(argc==4)
		{
			ImageSource *is=ISLoadImage(argv[1]);
			is=new ImageSource_GaussianBlur(is,atof(argv[2]));
			TIFFSaver ts(argv[3],is,false);
			ProgressText p;
			ts.SetProgress(&p);
			ts.Save();
			delete is;	
		}
		if(argc==5)
		{
			ImageSource *is=ISLoadImage(argv[1]);
//			is=new ImageSource_Downsample(is,(is->width*95)/100,(is->height*95)/100);
//			is=new ImageSource_LanczosSinc(is,atoi(argv[2]),atoi(argv[3]));
			int w=atoi(argv[2]);
			int h=atoi(argv[3]);
			float radius=float(is->width)/w;
//			cerr << "Using Gaussian radius of: " << radius << endl;
//			is=new ImageSource_GaussianBlur(is,radius);
			is=ISScaleImageBySize(is,w,h);
			TIFFSaver ts(argv[4],is,false);
			ProgressText p;
			ts.SetProgress(&p);
			ts.Save();
			delete is;	
		}
#if 0
		if(argc==2)
		{
			// Rescue routine for CM screwup!
			ImageSource *is=ISLoadImage(argv[1]);
			
			int prevrow[3]={-1,-1,-1};
			int row=0;
			while(row<is->height)
			{
				ISDataType *rowptr;
				do
				{
					rowptr=is->GetRow(row);
					++row;
				} while((row<is->height)
					&& (rowptr[0]==prevrow[0])
					&& (rowptr[1]==prevrow[1])
					&& (rowptr[2]==prevrow[2]));
				if(row<is->height)
				{
					cerr << "Unique row at " << row << endl;
					prevrow[0]=rowptr[0];
					prevrow[1]=rowptr[1];
					prevrow[2]=rowptr[2];
					int x=0;
					int tmp[3]={-1,-1,-1};
					while(x<is->width)
					{
						int s=x*is->samplesperpixel;
						if(rowptr[s]!=tmp[0] || rowptr[s+1]!=tmp[1] || rowptr[s+2]!=tmp[2])
						{
							tmp[0]=rowptr[s];
							tmp[1]=rowptr[s+1];
							tmp[2]=rowptr[s+2];
							float r=(100.0 * tmp[0])/IS_SAMPLEMAX;
							float g=(100.0 * tmp[1])/IS_SAMPLEMAX;
							float b=(100.0 * tmp[2])/IS_SAMPLEMAX;
							cout << r << ", " << g << ", " << b << endl;
							cerr << r << ", " << g << ", " << b << endl;
						}
						++x;
					}
				}
			}
			delete is;	
		}
#endif
	}
	catch(const char *err)
	{
		cerr << "Error: " << err << endl;
	}
	return(0);
}
