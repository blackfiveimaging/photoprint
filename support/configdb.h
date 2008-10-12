/*
 * configdb.h - classes to simplify configuration file handling
 *
 * Copyright (c) 2004, 2005 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *

In its simplest form you use it like this:

#include "configdb.h"

class MyClassThatNeedsConfigData : public ConfigDB
{
	public:
	MyClassThatNeedsConfigData(ConfigFile *myfile)
		: ConfigDB(Template);
	{
		new ConfigDBHandler(myfile,"[SectionName]",this);
		// (This object is owned by the ConfigFile and will be freed by it.)
	}
	void MemberFunction()
	{
		cout << "Int value: " << FindInt("AnIntValue") << endl;
	}
	private:
	static ConfigTemplate Template[];
};

ConfigTemplate MyClassThatNeedsConfigData::Template[]=
{
	ConfigTemplate("AnIntValue",int(17)), // Default value is 17
	ConfigTemplate("AStringValue","Default"),
	ConfigTemplate("AFloatValue",float(3.5)),
	ConfigTemplate() // NULL terminated...
};


int main(int argc,char **argv)
{
	ConfigFile myconfig;
	MyClassThatNeedsConfigData myclass(myconfig);
	myconfig.ParseConfigFile("/path/to/myconfigfile");

	myclass.MemberFunction();
	cout "String value: " << myclass.FindString("AStringValue");

	myclass.SetFloat("AFloatValue",1.45);
	
	myconfig.SaveConfigFile("/path/to/myconfigfile");

	return(0);
}

 */

#ifndef CONFIGDB_H
#define CONFIGDB_H

enum ConfigArgType {ConfigARG_UNKNOWN,ConfigARG_STRING,ConfigARG_INTEGER,ConfigARG_FLOAT};

class ConfigTemplate
{
	public:
	ConfigTemplate(const char *name, const char *val) : Name(name), Type(ConfigARG_STRING), defstring(val)
	{
	}
	ConfigTemplate(const char *name, int val) : Name(name), Type(ConfigARG_INTEGER), defint(val)
	{
	}
	ConfigTemplate(const char *name, double val) : Name(name), Type(ConfigARG_FLOAT), deffloat(val)
	{
	}
	ConfigTemplate() : Type(ConfigARG_UNKNOWN)
	{
	}
	const char *Name;
	enum ConfigArgType Type;
	const char *defstring;
	int defint;
	double deffloat;
};


class ConfigOption;


class ConfigDB
{
	public:
	ConfigDB(struct ConfigTemplate *templ);
	virtual ~ConfigDB();
	int ParseString(const char *string);
	virtual void SaveDB(FILE *file);
	enum ConfigArgType QueryType(const char *Name);
	const char *FindString(const char *Name);
	int FindInt(const char *Name);
	double FindFloat(const char *Name);
	void SetInt(const char *Name,int val);
	void SetFloat(const char *Name,double val);
	void SetString(const char *Name,const char *val);
	private:
	ConfigOption *FindOption(const char *Name);
	struct ConfigOption *firstopt;
	friend class ConfigOption;
};


class ConfigOption
{
	public:
	ConfigOption(ConfigDB *db,struct ConfigTemplate *templ);
	~ConfigOption();
	private:
	const char *Name;
	enum ConfigArgType Type;
	union
	{
		char *string;
		int intnumber;
		double floatnumber;
	} Value;
	ConfigOption *next;
	ConfigOption *prev;
	ConfigDB *db;
	friend class ConfigDB;
};

class ConfigFile;
class ConfigSectionHandler
{
	public:
	ConfigSectionHandler(ConfigFile *inifile,const char *section);
	virtual ~ConfigSectionHandler();
	virtual void SelectSection();
	virtual void LeaveSection();
	virtual void ParseString(const char *string)=0;
	virtual void SaveSection(FILE *file)=0;
	const char *section;
	private:
	ConfigSectionHandler *next,*prev;
	ConfigFile *inifile;
	friend class ConfigFile;
};


class ConfigDBHandler : public ConfigSectionHandler
{
	public:
	ConfigDBHandler(ConfigFile *inf,const char *section,ConfigDB *db);
	~ConfigDBHandler();
	virtual void ParseString(const char *string);
	virtual void SaveSection(FILE *file);
	private:
	ConfigDB *db;
};


class ConfigFile
{
	public:
	ConfigFile();
	virtual ~ConfigFile();
	virtual void ParseConfigFile(const char *inifile);
	virtual bool SaveConfigFile(const char *inifile);
	private:
	ConfigSectionHandler *FindHandler(const char *section);
	ConfigSectionHandler *first;
	ConfigSectionHandler *current;
	friend class ConfigSectionHandler;
};

#endif
