#ifndef SEARCHPATHDBHANDLER_H
#define SEARCHPATHDBHANDLER_H

#include "configdb.h"

class SearchPathHandlerDBHandler : public ConfigDBHandler
{
	public:
	SearchPathHandlerDBHandler(ConfigFile *file,const char *section,ConfigDB *db,SearchPathHandler *sp,const char *pathparam)
		: ConfigDBHandler(file,section,db), db(db), sp(sp), pathparameter(pathparam)
	{
	}
	virtual ~SearchPathHandlerDBHandler()
	{
	}
	virtual void LeaveSection()
	{
		sp->ClearPaths();
		sp->AddPath(db->FindString(pathparameter));
		ConfigDBHandler::LeaveSection();
	}
	virtual void SaveSection(FILE *file)
	{
		char *p=sp->GetPaths();
		db->SetString(pathparameter,p);
		free(p);
		ConfigDBHandler::SaveSection(file);
	}
	protected:
	ConfigDB *db;
	SearchPathHandler *sp;
	const char *pathparameter;
};

#endif
