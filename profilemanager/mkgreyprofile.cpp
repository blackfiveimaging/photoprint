#include <iostream>

#include "profilemanager.h"

using namespace std;

// ProPhoto RGB
// 1.8 D50  	0.7347  	0.2653  	0.1596  	0.8404  	0.0366  	0.0001

int main(int argc,char **argv)
{
	if(argc==2)
	{
		CMSWhitePoint wp(6500);
		CMSGamma greygam(2.2);
		CMSProfile p(greygam,wp); // Grey Profile
		p.Save(argv[1]);
	}
	return(0);
}
