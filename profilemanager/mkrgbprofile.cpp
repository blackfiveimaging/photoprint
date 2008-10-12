#include <iostream>

#include "profilemanager.h"

using namespace std;

// ProPhoto RGB
// 1.8 D50  	0.7347  	0.2653  	0.1596  	0.8404  	0.0366  	0.0001

int main(int argc,char **argv)
{
	if(argc==2)
	{
		CMSWhitePoint wp(5000);
		CMSRGBPrimaries prim(0.7347,0.2653 , 0.1596,0.8404 , 0.0366,0.0001);
		CMSRGBGamma gam(1.1);
		CMSProfile p(prim,gam,wp); // RGB Profile
		p.Save(argv[1]);
	}
	return(0);
}
