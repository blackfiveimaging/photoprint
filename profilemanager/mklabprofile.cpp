#include <iostream>

#include "profilemanager.h"

using namespace std;


int main(int argc,char **argv)
{
	if(argc==2)
	{
		CMSWhitePoint wp(5000);
		CMSProfile p(wp);
		p.Save(argv[1]);
	}
	return(0);
}
