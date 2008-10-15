/*
 * util.cpp - miscellaneous support functions.
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "support/searchpath.h"
#include "support/pathsupport.h"

using namespace std;

bool CheckSettingsDir(const char *dirname)
{
//	const char *homedir=getenv("HOME");
	const char *homedir=get_homedir();
	if(homedir)
	{
		char *path=(char *)malloc(strlen(homedir)+strlen(dirname)+2);
		sprintf(path,"%s%c%s",homedir,SEARCHPATH_SEPARATOR,dirname);

		cerr << "Settings directory: " << path << endl;
		
		struct stat s;
		if(stat(path,&s)!=0)
		{
			cerr << "Need to create directory... " << errno << endl;
			if(errno==ENOENT)
#ifdef WIN32
				mkdir(path);
#else
				mkdir(path,0755);
#endif
		}
		free(path);
		return(true);
	}
	else
		return(false);
}


char *BuildAbsoluteFilename(const char *fname)
{
	char *result=NULL;
	char cwdbuf[1024];
	int l;

	getcwd(cwdbuf,1023);

	l=strlen(fname)+strlen(cwdbuf)+3;	
	result=(char *)malloc(l);
	
	sprintf(result,"%s%c%s",cwdbuf,SEARCHPATH_SEPARATOR,fname);	
	return(result);
}


char *BuildFilename(const char *root,const char *suffix,const char *fileext)
{
	/* Build a filename like <imagename><channel>.<extension> */
	char *extension;

	char *filename=(char *)malloc(strlen(root)+strlen(suffix)+strlen(fileext)+3);

	char *root2=strdup(root);
	extension = root2 + strlen (root2) - 1;
	while (extension >= root2)
	{
		if (*extension == '.') break;
		extension--;
	}
	if (extension >= root2)
	{
		*(extension++) = '\0';
		sprintf(filename,"%s%s.%s", root2, suffix, fileext);
	}
	else
		sprintf(filename,"%s%s", root2, suffix);
	free(root2);

	return(filename);
}


char *SerialiseFilename(const char *fname,int serialno,int max)
{
	int digits=0;
	while(max)
	{
		++digits;
		max/=10;
	}
	char *ftmp=strdup(fname);
	const char *extension="";
	int idx=strlen(ftmp)-1;
	while(idx>0)
	{
		if(ftmp[idx]=='.')
			break;
		--idx;
	}
	if(idx)
	{
		extension=ftmp+idx+1;
		ftmp[idx]=0;
	}

	char *result=(char *)malloc(strlen(ftmp)+strlen(extension)+digits+4);
	if(digits)
		sprintf(result,"%s_%0*d.%s",ftmp,digits,serialno,extension);
	else
		sprintf(result,"%s_%d.%s",ftmp,serialno,extension);
	free(ftmp);
	return(result);
}



int TestNumeric(char *str)
{
	int result=1;
	int c;
	while((c=*str++))
	{
		if((c<'0')||(c>'9'))
			result=0;
	}
	return(result);
}


int TestHostName(char *str,char **hostname,int *port)
{
	int result=0;
	int c;
	char *src=str;
	while((c=*src++))
	{
		if(c==':')
		{
			if(TestNumeric(src))
			{
				int hnl=src-str;
				*port=atoi(src);
				*hostname=(char *)malloc(hnl+1);
				strncpy(*hostname,str,hnl);
				(*hostname)[hnl-1]=0;
				result=1;	
			}
		}

	}
	return(result);
}


bool CompareFiles(const char *fn1,const char *fn2)
{
	bool result=true;
	int l1,l2;
	char *buf1,*buf2;
	ifstream i1,i2;
 	i1.open(fn1,ios::binary);
	i2.open(fn2,ios::binary);

	i1.seekg(0, ios::end);
	l1= i1.tellg();
	i1.seekg (0, ios::beg);

	i2.seekg(0, ios::end);
	l2= i2.tellg();
	i2.seekg (0, ios::beg);

	if(l1==l2)
	{
		buf1 = new char [l1];
		buf2 = new char [l2];

		i1.read (buf1,l1);
		i2.read (buf2,l2);

		for(int i=0;i<l1;++i)
		{
			if(buf1[i]!=buf2[i])
			{
				result=false;
				i=l1;
			}
		}
		delete[] buf1;
		delete[] buf2;
	}
	else
		result=false;
	i1.close();
	i2.close();

	return(result);
}

