#include <iostream>

#include <gtk/gtk.h>

#include "pathsupport.h"

using namespace std;

int main(int argc,char **argv)
{
	gtk_init(&argc,&argv);

	char *op="$home/.photoprint";
	if(argc>1)
		op=argv[1];

	char *np=substitute_homedir(op);
	cerr << op << " -> " << np << endl;
	free(np);
	return(0);
}
