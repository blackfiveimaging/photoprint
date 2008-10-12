#include <iostream>

#include "convkernel_gaussian_1D.h"

using namespace std;

int main(int argc,char **argv)
{
	float r=2.0;
	if(argc==2)
		r=atof(argv[1]);
	ConvKernel_Gaussian_1D ck(r);
//	cerr << ck << endl;
	ck.Normalize();
	cerr << ck << endl;
}
