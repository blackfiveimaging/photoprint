#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

int main(int argc,char **argv)
{
	char *oldlocale,*savedlocale,*result;
	setlocale(LC_ALL,"");

	printf("Setting locale to 'C'\n");

	oldlocale=setlocale(LC_ALL,NULL);
    savedlocale=strdup(oldlocale);
	printf("Old locale setting: %s\n",oldlocale);

	result=setlocale(LC_ALL,"C");

	printf("Result of setlocale: %s\n",result);
	printf("Old locale setting: %s\n",oldlocale);

	setlocale(LC_ALL,savedlocale);
	free(savedlocale);

	return(0);
}
