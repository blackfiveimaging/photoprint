/*
 * configdb.cpp - classes to simplify configuration file handling
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "configdb.h"

using namespace std;

static void StripNewline(char *in)
{
	int l=strlen(in)-1;    
	while((l>0) && ((in[l]=='\n') || (in[l]=='\r')))        
		--l;
	in[l+1]=0;
}


ConfigOption::~ConfigOption()
{
	switch(Type)
	{
		case ConfigARG_STRING:
			free(Value.string);
			break;
		default:
			break;
	}
		
	if(next)
		next->prev=prev;
	if(prev)
		prev->next=next;
	else
		db->firstopt=next;
}


ConfigOption::ConfigOption(ConfigDB *db,struct ConfigTemplate *templ)
	: next(NULL), prev(NULL), db(db)
{
	Name=templ->Name;
	Type=templ->Type;
	Value.string=NULL;
	switch(Type)
	{
		case ConfigARG_STRING:
			if(templ->defstring)
				Value.string=strdup(templ->defstring);
			break;
			
		case ConfigARG_INTEGER:
			Value.intnumber=templ->defint;
			break;
				
		case ConfigARG_FLOAT:
			Value.floatnumber=templ->deffloat;
			break;

		default:
			break;
	}
	if((prev=db->firstopt))
	{
		while(prev->next)
			prev=prev->next;
		prev->next=this;
	}
	else
		db->firstopt=this;
};


ConfigDB::~ConfigDB()
{
	while(firstopt)
		delete firstopt;
}


int ConfigDB::ParseString(const char *string)
{
	char *in;
	char *string2;
	int l;
	while(*string==' ')
		++string;
	
	in=string2=strdup(string);
	l=strlen(in)-1;
	while((l>0) && ((in[l]=='\n') || (in[l]=='\r')))
		--l;
	in[l+1]=0;
	
	while(1)
	{
		switch (*in)
		{
			case '\0':
				free(string2);
				return(0);
				break;
			case '=':
				*in='\0';
				++in;
				if(*in)
				{
					struct ConfigOption *opt;
					if((opt=FindOption(string2)))
					{
						switch(opt->Type)
						{
							case ConfigARG_STRING:
								if(opt->Value.string)
									free(opt->Value.string);
								opt->Value.string=strdup(in);
								break;
							case ConfigARG_INTEGER:
								opt->Value.intnumber=atoi(in);
								break;
							case ConfigARG_FLOAT:
								opt->Value.floatnumber=atof(in);
								break;
							default:
								cerr << "Error: Unknown type for option: " << opt->Name << endl;
								break;
						}
						free(string2);
						return(1);
					}
				}
				free(string2);
				return(0);
				break;
		}
		++in;
	}
}


struct ConfigOption *ConfigDB::FindOption(const char *Name)
{
	struct ConfigOption *opt=firstopt;
	struct ConfigOption *result=NULL;

	while(opt)
	{
		if(strcasecmp(opt->Name,Name)==0)
		{
			result=opt;
			opt=NULL;
		}
		else
			opt=opt->next;
	}
	if(!result)
		cerr << "Warning: option " << Name << " not found" << endl;
	return(result);
}


const char *ConfigDB::FindString(const char *Name)
{
	struct ConfigOption *opt;
	if((opt=FindOption(Name)))
	{
		if(opt->Type==ConfigARG_STRING)
			return(opt->Value.string);
		else
			cerr << "Error: " << Name << " is not a string option" << endl;
		return(NULL);
	}
	return(NULL);
}


int ConfigDB::FindInt(const char *Name)
{
	struct ConfigOption *opt;
	if((opt=FindOption(Name)))
	{
		if(opt->Type==ConfigARG_INTEGER)
		{
			return(opt->Value.intnumber);
		}
		else
			cerr << "Error: " << Name << " is not an integer option" << endl;
	}
	else
		cerr << Name << " Not found..." << endl;

	return(0);
}


double ConfigDB::FindFloat(const char *Name)
{
	struct ConfigOption *opt;
	if((opt=FindOption(Name)))
	{
		if(opt->Type==ConfigARG_FLOAT)
		{
			return(opt->Value.floatnumber);
		}
		else
			cerr << "Error: " << Name << " is not a float option" << endl;
	}
	else
		cout << "Not found..." << endl;

	return(0);
}


void ConfigDB::SetInt(const char *Name,int val)
{
	struct ConfigOption *opt;
	if((opt=FindOption(Name)))
	{
		if(opt->Type==ConfigARG_INTEGER)
		{
			opt->Value.intnumber=val;
		}
		else
			cerr << "Error: " << Name << " is not an integer option" << endl;
	}
	else
		cout << "Not found..." << endl;
}


void ConfigDB::SetFloat(const char *Name,double val)
{
	struct ConfigOption *opt;
	if((opt=FindOption(Name)))
	{
		if(opt->Type==ConfigARG_FLOAT)
		{
			opt->Value.floatnumber=val;
		}
		else
			cerr << "Error: " << Name << " is not a float option" << endl;
	}
	else
		cout << "Not found..." << endl;
}


void ConfigDB::SetString(const char *Name,const char *val)
{
	struct ConfigOption *opt;
	if((opt=FindOption(Name)))
	{
		if(opt->Type==ConfigARG_STRING)
		{
			if(opt->Value.string==val)
				return;
			if(opt->Value.string)
				free(opt->Value.string);
			if(val)
				opt->Value.string=strdup(val);
			else
				opt->Value.string=NULL;
		}
		else
			cerr << "Error: " << Name << " is not an integer option" << endl;
	}
	else
		cout << "Not found..." << endl;
}


enum ConfigArgType ConfigDB::QueryType(const char *Name)
{
	struct ConfigOption *opt;
	if((opt=FindOption(Name)))
		return(opt->Type);
	else
		return(ConfigARG_UNKNOWN);
}


void ConfigDB::SaveDB(FILE *file)
{
	ConfigOption *o=firstopt;
	while(o)
	{
		switch(o->Type)
		{
			case ConfigARG_INTEGER:
				fprintf(file,"%s=%d\n",o->Name,o->Value.intnumber);
				break;
			case ConfigARG_FLOAT:
				fprintf(file,"%s=%f\n",o->Name,o->Value.floatnumber);
				break;
			case ConfigARG_STRING:
				if(o->Value.string && strlen(o->Value.string))
					fprintf(file,"%s=%s\n",o->Name,o->Value.string);
				else
					fprintf(file,"# %s=\n",o->Name);
				break;
			default:
				fprintf(file,"# Error: bad type for option %s\n",o->Name);
				break;
		}
		o=o->next;
	}
}


ConfigDB::ConfigDB(struct ConfigTemplate *templ)
{
	firstopt=NULL;

	while(templ->Type!=ConfigARG_UNKNOWN)
	{
		new ConfigOption(this,templ);
		templ++;
	}
}


ConfigSectionHandler::ConfigSectionHandler(ConfigFile *inifile,const char *section)
	: section(section), next(NULL), inifile(inifile)
{
	if((prev=inifile->first))
	{
		while(prev->next)
			prev=prev->next;
		prev->next=this;
	}
	else
		inifile->first=this;
}


ConfigSectionHandler::~ConfigSectionHandler()
{
	if(next)
		next->prev=prev;
	if(prev)
		prev->next=next;
	else
		inifile->first=next;
}


void ConfigSectionHandler::SelectSection()
{
}


void ConfigSectionHandler::LeaveSection()
{
}


ConfigFile::ConfigFile() : first(NULL), current(NULL)
{
}


ConfigFile::~ConfigFile()
{
	while(first)
		delete first;
}


ConfigSectionHandler *ConfigFile::FindHandler(const char *section)
{
	ConfigSectionHandler *sh=first;
	while(sh)
	{
		int l=strlen(sh->section);
		if(strncmp(sh->section,section,l)==0)
		{
			return(sh);
		}
		sh=sh->next;
	}
	return(NULL);
}


void ConfigFile::ParseConfigFile(const char *inifile)
{
	FILE *file;
	if((file=fopen(inifile,"r")))
	{
		char inb[4096];
		while((fgets(inb,4096,file)))
		{
			StripNewline(inb);
			if((strlen(inb)>2) && inb[0]!='#')
			{
				if(inb[0]=='[')
				{
					if(current)
						current->LeaveSection();

					current=FindHandler(inb);

					if(current)
						current->SelectSection();
				}
				else if(current)
					current->ParseString(inb);
			}
		}
		if(current)
			current->LeaveSection();
        fclose(file);
	}
}


bool ConfigFile::SaveConfigFile(const char *inifile)
{
	FILE *file;
	if((file=fopen(inifile,"w")))
	{
		ConfigSectionHandler *h;
		h=first;
		while(h)
		{
			fprintf(file,"\n%s\n",h->section);
			h->SaveSection(file);
			h=h->next;
		}
		fclose(file);
		return(true);
	}
	else
		return(false);
}


ConfigDBHandler::ConfigDBHandler(ConfigFile *inf,const char *section,ConfigDB *db)
	: ConfigSectionHandler(inf,section), db(db)
{
}


ConfigDBHandler::~ConfigDBHandler()
{
}


void ConfigDBHandler::ParseString(const char *string)
{
	db->ParseString(string);
}


void ConfigDBHandler::SaveSection(FILE *file)
{
	db->SaveDB(file);
}
