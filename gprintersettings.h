#ifndef GPRINTERSETTINGS_H
#define GPRINTERSETTINGS_H

#include <gutenprint/gutenprint.h>

#include "support/pageextent.h"
#include "printoutput.h"

class GPrinterSettings : public ConfigSectionHandler, public PageExtent
{
	public:
	GPrinterSettings(PrintOutput &output,ConfigFile *inf,const char *section);
	~GPrinterSettings();
	void ParseString(const char *string);
	void SaveSection(FILE *file);
	void SelectSection();
	bool SetDriver(const char *driver);
	void Validate();
	void Reset();
	void Dump();
	stp_vars_t *stpvars;
	protected:
	PrintOutput &output;
	private:
	bool initialised;
	bool ppdsizes_workaround_done;
};


#endif
