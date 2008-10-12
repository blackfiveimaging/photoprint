#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>

#include "pathsupport.h"

#include "searchpath.h"

using namespace std;

class SearchPathInstance
{
	public:
	SearchPathInstance(SearchPathHandler *header,const char *path);
	~SearchPathInstance();
	char *Simplify(const char *file);
	char *MakeAbsolute(const char *file);
	SearchPathInstance *Next();
	protected:
	char *path;
	SearchPathInstance *next,*prev;
	SearchPathHandler *header;
	friend class SearchPathHandler;
	friend std::ostream& operator<<(std::ostream &s,SearchPathInstance &sp);
};


SearchPathInstance::SearchPathInstance(SearchPathHandler *header,const char *path)
	: path(NULL), next(NULL), prev(NULL), header(header)
{
	this->path=substitute_homedir(path);
	prev=header->first;
	if(prev)
	{
		while(prev->next)
			prev=prev->next;
		prev->next=this;
	}
	else
		header->first=this;
}


SearchPathInstance::~SearchPathInstance()
{
	if(next)
		next->prev=prev;
	if(prev)
		prev->next=next;
	else
		header->first=next;
	free(path);
}


char *SearchPathInstance::Simplify(const char *file)
{
	if(path && file)
	{
		if(strncmp(file,path,strlen(path))==0)
		{
			int i=strlen(path);
			if(file[i]==SEARCHPATH_SEPARATOR)
				++i;
			if(file[i])
				return(strdup(file+i));
		}
	}
	return(strdup(file));
}


char *SearchPathInstance::MakeAbsolute(const char *file)
{
	char *result=NULL;
	int l=strlen(path);
	int m=strlen(file);
	if(l&&m)
	{
		result=(char *)malloc(l+m+2);
		if(path[l-1]!=SEARCHPATH_SEPARATOR)
			sprintf(result,"%s%c%s",path,SEARCHPATH_SEPARATOR,file);
		else
			sprintf(result,"%s%s",path,file);
	}
	return(result);
}


SearchPathInstance *SearchPathInstance::Next()
{
	return(next);
}


std::ostream& operator<<(std::ostream &s,SearchPathInstance &spi)
{
	const char *homedir=get_homedir();
	char *path=spi.path;
	if(homedir && strncmp(homedir,path,strlen(homedir))==0)
	{
		s<<"$HOME";
		s<<path+strlen(homedir);
	}
	else
		s<<spi.path;
	return(s);
}


// SearchPathHandler


SearchPathHandler::SearchPathHandler()
	:	first(NULL), searchdirectory(NULL), searchfilename(NULL), searchiterator(NULL)
{
}


SearchPathHandler::~SearchPathHandler()
{
	while(first)
		delete first;

	if(searchdirectory)
		closedir(searchdirectory);

	if(searchfilename)
		free(searchfilename);
}


char *SearchPathHandler::Search(const char *file)
{
	struct stat statbuf;
	SearchPathInstance*spi=first;
	while(spi)
	{
		char *p=spi->MakeAbsolute(file);
//		cerr << file << " -> " << p << endl;

		if(stat(p,&statbuf)==0)
			return(p);
		free(p);

		spi=spi->Next();
	}

	if(stat(file,&statbuf)==0)
		return(strdup(file));

	return(NULL);
}


void SearchPathHandler::AddPath(const char *path)
{
	if(path)
	{
		try
		{
			char *p=strdup(path);
			char *p2=p;
			char *p3=p;
			while(*p3)
			{
				if(*p3==SEARCHPATH_DELIMITER_C)
				{
					*p3=0;
					new SearchPathInstance(this,p2);
					p2=p3+1;
				}
				++p3;
			}
			new SearchPathInstance(this,p2);
			free(p);
		}
		catch(const char *err)
		{
			cerr << "Error: " << err << endl;
		}
	}
}


SearchPathInstance *SearchPathHandler::FindPath(const char *path)
{
	SearchPathInstance *result=NULL;
	char *p=NULL;

	if(path)
	{
		if(strncmp(path,"$HOME",5)==0)
		{
			char *homedir=getenv("HOME");
			if(!homedir)
				throw "No home directory";
			p=(char *)malloc(strlen(path)-5+strlen(homedir)+2);
			sprintf(p,"%s%s",homedir,path+5);
		}
		else
			p=strdup(path);

		SearchPathInstance *iter=first;
		while(iter)
		{
			if(strcmp(iter->path,p)==0)
				result=iter;
			iter=iter->Next();
		}
		free(p);
	}
	return(result);
}


void SearchPathHandler::RemovePath(const char *path)
{
	SearchPathInstance *spi=FindPath(path);
	if(spi)
		delete spi;
}


void SearchPathHandler::ClearPaths()
{
	while(first)
		delete first;
}


char *SearchPathHandler::MakeRelative(const char *path)
{
	char *best=NULL;
	unsigned int bestlen=100000;

	if(!path)
		return(NULL);

	SearchPathInstance *spi=first;
	while(spi)
	{
		char *rel=spi->Simplify(path);
		if(strlen(rel)<bestlen)
		{
			if(best)
				free(best);
			best=rel;
			bestlen=strlen(best);
		}
		else
			free(rel);
		spi=spi->Next();
	}
	if(!best)
		best=strdup(path);
	return(best);
}


const char *SearchPathHandler::GetNextFilename(const char *last)
{
	if(searchfilename)
		free(searchfilename);
	searchfilename=NULL;

	// If we're provided with a NULL pointer, clean up
	// the remnants of any previous run...

	if(!last)
	{
		if(searchdirectory)
			closedir(searchdirectory);
		searchdirectory=NULL;

		if((searchiterator=first))
		{
			while(!searchdirectory)
			{
				if(!(searchdirectory=opendir(searchiterator->path)))
					searchiterator=searchiterator->Next();
				if(!searchiterator)
					return(NULL);
			}
		}
	}

	struct dirent *de=NULL;

	while(searchdirectory && !de)
	{
		de=readdir(searchdirectory);
		if(!de)
		{
			closedir(searchdirectory);
			searchdirectory=NULL;
			while(!searchdirectory && searchiterator->Next())
			{
				searchiterator=searchiterator->Next();
				searchdirectory=opendir(searchiterator->path);
			}
			if(searchdirectory)
				de=readdir(searchdirectory);
		}
		if(de)
		{
			if(strcmp(".",de->d_name)==0)
				de=NULL;
			else if(strcmp("..",de->d_name)==0)
				de=NULL;
		}
	}
	if(de)
	{
		searchfilename=strdup(de->d_name);
	}
	return(searchfilename);
}


const char *SearchPathHandler::GetNextPath(const char *last)
{
	const char *result=NULL;

	if(!last)
		searchiterator=first;

	if(searchiterator)
	{
		result=searchiterator->path;
		searchiterator=searchiterator->Next();
	}
	return(result);
}


std::ostream& operator<<(std::ostream &s,SearchPathHandler &sp)
{
	SearchPathInstance *spi=sp.first;
	while(spi)
	{
		s << *spi;
		spi=spi->Next();
		if(spi)
			s << SEARCHPATH_DELIMITER_S;
	}
	return(s);
}


char *SearchPathHandler::GetPaths()
{
	const char *homedir=get_homedir();
	int homedirlen=0;
	int sl=0;

	if(homedir)
		homedirlen=strlen(homedir);

	SearchPathInstance *spi=first;
	while(spi)
	{
		if(homedir && strncmp(homedir,spi->path,homedirlen)==0)
			sl+=strlen(spi->path)+strlen("$HOME/")+1-homedirlen;
		else
			sl+=strlen(spi->path)+1;
		spi=spi->Next();
	}

	char *result=(char *)malloc(sl+1);
	result[0]=0;

	spi=first;
	while(spi)
	{
		if(homedir && strncmp(homedir,spi->path,homedirlen)==0)
		{
			strcat(result,"$HOME");
			strcat(result,spi->path+homedirlen);
			spi=spi->Next();
			if(spi)
				strcat(result,SEARCHPATH_DELIMITER_S);
		}
		else
		{
			strcat(result,spi->path);
			spi=spi->Next();
			if(spi)
				strcat(result,SEARCHPATH_DELIMITER_S);
		}
	}
	return(result);
}
