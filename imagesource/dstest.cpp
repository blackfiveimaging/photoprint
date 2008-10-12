#include <iostream>

using namespace std;

// * * * * * * * * * * * * * * * * * * * *|* * * * * * * * * * * * * * * * * * * *|* * * * * * * * * * * * * * * * * * * *|
// * * * * * * * * * * * * * *|* * * * * * * * * * * * * *|* * * * * * * * * * * * * *|* * * * * * * * * * * * * *|* * * * * * * * * * * * * *|

int srcdata[]={
	400,900,1500,300,265,123,127,184,236,275,521,952,653,190,129,395,390,379,410,610,610,
	400,900,1500,300,265,123,127,184,236,275,521,952,653,190,129,395,390,379,410,610,610,
	400,900,1500,300,265,123,127,184,236,275,521,952,653,190,129,395,390,379,410,610,610,
	400,900,1500,300,265,123,127,184,236,275,521,952,653,190,129,395,390,379,410,610,610,
	400,900,1500,300,265,123,127,184,236,275,521,952,653,190,129,395,390,379,410,610,610,
	400,900,1500,300,265,123,127,184,236,275,521,952,653,190,129,395,390,379,410,610,610,
	400,900,1500,300,265,123,127,184,236,275,521,952,653,190,129,395,390,379,410,610,610,
	400,900,1500,300,265,123,127,184,236,275,521,952,653,190,129,395,390,379,410,610,610,
	400,900,1500,300,265,123,127,184,236,275,521,952,653,190,129,395,390,379,410,610,610,
	400,900,1500,300,265,123,127,184,236,275,521,952,653,190,129,395,390,379,410,610,610,
	400,900,1500,300,265,123,127,184,236,275,521,952,653,190,129,395,390,379,410,610,61,
};
int dstdata[17]={0};

int main(int argc,char **argv)
{
	int srcw=sizeof(srcdata)/sizeof(int)-1;
	int dstw=sizeof(dstdata)/sizeof(int);
	for(int i=0;i<dstw;++i) dstdata[i]=0;
	cerr << "Source width; " << srcw << endl;
	cerr << "Dest width: " << dstw << endl;

	int a=0;
	int src=0;
	int dst=0;
	double pixel=0.0;
	while(dst<=dstw)
	{
		a+=dstw;
		while(a<srcw)
		{
			cerr << "a: " << a << " - storing whole sample: 1.0, src:" << src << endl;
			pixel+=srcdata[src++];
			a+=dstw;
		}
		float p=srcw-(a-dstw);
		a-=srcw;
		p/=dstw;
		cerr << "a: " << a <<" - storing partial sample: " << p << ", src: " << src << endl;
		pixel+=p*srcdata[src];
		cerr << "Writing sample: " << dst << endl;
		dstdata[dst++]=0.5+(pixel*dstw)/srcw;
		cerr << "Seeding with partial sample: " << 1.0-p << ", src: " << src <<  endl;
		pixel=(1.0-p)*srcdata[src++];
	}

	for(int i=0;i<dstw;++i)
	{
		cerr << dstdata[i] << ", ";
	}
	cerr << endl;

	double mean=0.0;
	for(int i=0;i<srcw;++i)
		mean+=srcdata[i];
	cerr << "Source mean: " << mean/srcw << endl;
	mean=0.0;
	for(int i=0;i<dstw;++i)
		mean+=dstdata[i];
	cerr << "Source mean: " << mean/dstw << endl;
	return(0);
}

