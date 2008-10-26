/*
 * gprintersettings.cpp - Bridges GutenPrint's stp_vars_t and the configuration file.
 * A subclass of ConfigSectionHandler.
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <iostream>

#include <string.h>
#include <glib.h>
#include <glib/gprintf.h>

#include "stp_support/stputil.h"
#include "gprintersettings.h"
#include "support/util.h"

#include "support/generaldialogs.h"

#define DEFAULT_PPD_STRING "<Default Queue PPD>"

using namespace std;


static void dumpstrings(const stp_parameter_t *desc)
{
	int j,strcount;
	stp_string_list_t *strlist=desc->bounds.str;
	if(strlist)
	{
		strcount=stp_string_list_count(strlist);
		for(j=0;j<strcount;++j)
		{
			stp_param_string_t *p=stp_string_list_param(strlist,j);
			fprintf(stderr,"   %s (%s)\n",p->text,p->name);
		}
	}
	else
		fprintf(stderr,"  No string list!\n");
}


static void dumpvars(stp_vars_t *v)
{
	stp_parameter_list_t params = stp_get_parameter_list(v);
	
	int count = stp_parameter_list_count(params);
	
	for (int i = 0; i < count; i++)
	{
		const stp_parameter_t *p = stp_parameter_list_param(params, i);
//		if((p->p_level<=STP_PARAMETER_LEVEL_ADVANCED4)
//			&& ((p->p_class==STP_PARAMETER_CLASS_FEATURE) || (p->p_class==STP_PARAMETER_CLASS_OUTPUT)))
		{
			stp_parameter_t desc;
			stp_describe_parameter(v,p->name,&desc);
			const char *str;
			int ival;
			double fval;

			fprintf(stderr,"%s, %d, %d, %d, %d ",desc.name,desc.p_type,p->p_type,desc.p_class,desc.p_level);
			
			if(desc.is_active)
			{
				fprintf(stderr,", Active");
			}
			else
			{
				fprintf(stderr,", Inactive");
			}
				switch(desc.p_type)
				{
					case STP_PARAMETER_TYPE_STRING_LIST:
						str=stp_get_string_parameter(v,desc.name);
						fprintf(stderr,", StringList\n");
						dumpstrings(&desc);
						if(str)
							fprintf(stderr,"  Currently set to: %s\n",str);
						break;

					case STP_PARAMETER_TYPE_INT:
						ival=stp_get_int_parameter(v,desc.name);
						fprintf(stderr,", Int\n");
						fprintf(stderr,"  Currently set to: %d\n",ival);
						break;

					case STP_PARAMETER_TYPE_BOOLEAN:
						ival=stp_get_boolean_parameter(v,desc.name);
						fprintf(stderr,", Boolean\n");
						fprintf(stderr,"  Currently set to: %d\n",ival);
						break;

					case STP_PARAMETER_TYPE_DOUBLE:
						fval=stp_get_float_parameter(v,desc.name);
						fprintf(stderr,", Double\n");
						fprintf(stderr,"  Currently set to: %f\n",fval);
						break;

					default:
						fprintf(stderr,", Other\n");
						break;					
			}
		}
	}
	stp_parameter_list_destroy(params);
}


void GPrinterSettings::Dump()
{
	dumpvars(stpvars);
}


GPrinterSettings::GPrinterSettings(PrintOutput &output,ConfigFile *inf,const char *section)
	: ConfigSectionHandler(inf,section), PageExtent(), stpvars(NULL), output(output),
	initialised(false), ppdsizes_workaround_done(false)
{
#if 0
	cerr << "In GPrinterSetting constructor - about to call stp_init()" << endl;
	stp_init();
	cerr << "Done" << endl;
#endif
	stpvars=stp_vars_create();
}


GPrinterSettings::~GPrinterSettings()
{
	if(stpvars)
		stp_vars_destroy(stpvars);
}


void GPrinterSettings::SelectSection()
{
	/* Slight hack here: The printer driver is stored in the [Output] section
	   which is prior to the [Print] section.  Clearing this forces the driver
	   to be set before the printer options are set */

	initialised=false;
}


void GPrinterSettings::ParseString(const char *string)
{
	if(!initialised)
	{
		const char *driver=output.FindString("Driver");
		if(!SetDriver(driver))
			output.SetString("Driver",DEFAULT_PRINTER_DRIVER);
		initialised=true;
	}

	while(*string==' ')
		++string;
	
	char *value;
	char *token;
	value=token=strdup(string);
	int l=strlen(token)-1;
	while((l>0) && ((token[l]=='\n') || (token[l]=='\r')))
		--l;
	token[l+1]=0;

	while(1)
	{
		switch (*value)
		{
			case '\0':
				free(token);
				return;
				break;
			case '=':
				*value='\0';
				++value;
				if(*value)
				{
					if(strcmp("CustomWidth",token)==0)
					{
						cerr << "Setting custom width to: " << value << endl;
						stp_set_page_width(stpvars,pagewidth=atoi(value));
					}
					else if(strcmp("CustomHeight",token)==0)
					{
						cerr << "Setting custom height to: " << value << endl;
						stp_set_page_height(stpvars,pageheight=atoi(value));
					}					
					else
					{
						stp_parameter_t param;
						stp_describe_parameter(stpvars,token,&param);
						switch(param.p_type)
						{
							case STP_PARAMETER_TYPE_STRING_LIST:
								if(strcmp("PageSize",token)==0)
									cerr << "Setting PageSize to: " << value << endl;
								stp_set_string_parameter(stpvars,token,value);
								break;
		
							case STP_PARAMETER_TYPE_FILE:
								// Because the queue PPD filenames returned by CUPS
								// aren't persistent between sessions, we have to
								// save an escape string, and substitute the current
								// filename at preset loading time.
								if(strcmp(value,DEFAULT_PPD_STRING)==0)
								{
									cerr << "*** Fetching default PPD filename" << endl;
									char *defppd=output.GetPPD();
									if(defppd)
									{
										cerr << "Got default PPD filename: " << defppd << endl;
										stp_set_file_parameter(stpvars,token,defppd);
										free(defppd);

										stp_parameter_t desc2;
										stp_describe_parameter(stpvars,"PageSize",&desc2);
										cerr << "After setting PPD Default page size is now: " << desc2.deflt.str << endl;
										stp_set_string_parameter(stpvars,"PageSize",desc2.deflt.str);
										stp_parameter_description_destroy(&desc2);
										ppdsizes_workaround_done=true;
									}
									else
										cerr << "Couldn't get default PPD." << endl;
								}
								else
									stp_set_file_parameter(stpvars,token,value);
								break;

							case STP_PARAMETER_TYPE_INT:
	//							cerr << "Setting " << token << " to " << value << endl;
								stp_set_int_parameter(stpvars,token,atoi(value));
								break;
		
							case STP_PARAMETER_TYPE_BOOLEAN:
	//							cerr << "Setting " << token << " to " << value << endl;
								stp_set_boolean_parameter(stpvars,token,atoi(value));
								break;
		
							case STP_PARAMETER_TYPE_DOUBLE:
	//							cerr << "Setting " << token << " to " << value << endl;
								stp_set_float_parameter(stpvars,token,atof(value));
								break;
		
							default:
								break;
						}
						stp_parameter_description_destroy(&param);
					}
				}
				free(token);
				return;
				break;
		}
		++value;
	}
}


void GPrinterSettings::SaveSection(FILE *file)
{
	stp_parameter_list_t params = stp_get_parameter_list(stpvars);
	stp_parameter_t desc;

	int count = stp_parameter_list_count(params);
	
	// PPDFile parameter must be saved first, because it must be restored
	// before other parameters upon loading.
	// Because CUPS returns a local temporary filename for a queue's PPD
	// which is not persistent between sessions, we must save an escape
	// string to indicate that the queue's default should be used.

	stp_describe_parameter(stpvars,"PPDFile",&desc);
	if(desc.is_active)
	{
		cerr << "Saving PPDFile parameter..." << endl;
		const char *ppd=stp_get_file_parameter(stpvars,"PPDFile");
		char *defppd=output.GetPPD();
		cerr << "Current PPD: " << ppd << endl;
		cerr << "Default PPD: " << defppd << endl;
		if(defppd && CompareFiles(defppd,ppd))
			ppd=DEFAULT_PPD_STRING;
		if(defppd)
			free(defppd);
		fprintf(file,"PPDFile=%s\n",ppd);
	}

	for (int i = 0; i < count; i++)
	{
		const stp_parameter_t *p = stp_parameter_list_param(params, i);
		if((p->p_level<=STP_PARAMETER_LEVEL_ADVANCED4))
		{
			stp_parameter_t desc;
			stp_describe_parameter(stpvars,p->name,&desc);
			if(desc.is_active)
			{
				switch(desc.p_type)
				{
					case STP_PARAMETER_TYPE_STRING_LIST:
						{
							if(stp_check_string_parameter(stpvars,desc.name,STP_PARAMETER_DEFAULTED))
							{
								const char *str=stp_get_string_parameter(stpvars,desc.name);
//								if(!desc.is_mandatory || strcmp(str,desc.deflt.str)!=0)
									fprintf(file,"%s=%s\n",desc.name,str);
							}
						}
						break;

					case STP_PARAMETER_TYPE_INT:
						{
							if(stp_check_int_parameter(stpvars,desc.name,STP_PARAMETER_DEFAULTED))
							{
								int val=stp_get_int_parameter(stpvars,desc.name);
//								if(!desc.is_mandatory || val!=desc.deflt.integer)
									fprintf(file,"%s=%d\n",desc.name,val);
							}
						}
						break;

					case STP_PARAMETER_TYPE_BOOLEAN:
						{
							if(stp_check_boolean_parameter(stpvars,desc.name,STP_PARAMETER_DEFAULTED))
							{
								int val=stp_get_boolean_parameter(stpvars,desc.name);
//								if(!desc.is_mandatory || val!=desc.deflt.boolean)
									fprintf(file,"%s=%d\n",desc.name,val);
							}
						}
						break;

					case STP_PARAMETER_TYPE_DOUBLE:
						{
							if(stp_check_float_parameter(stpvars,desc.name,STP_PARAMETER_DEFAULTED))
							{
								double val=stp_get_float_parameter(stpvars,desc.name);
//								if(!desc.is_mandatory || val!=desc.deflt.dbl)
									fprintf(file,"%s=%f\n",desc.name,val);
							}
						}
						break;

					default:
						break;					
				}     
			}
			stp_parameter_description_destroy(&desc);
		}
	}

	fprintf(file,"CustomWidth=%d\n",pagewidth);
	fprintf(file,"CustomHeight=%d\n",pageheight);

	stp_parameter_list_destroy(params);
}


// SetDriver
// Returns false if the driver isn't recognised, true otherwise.

bool GPrinterSettings::SetDriver(const char *driver)
{
	bool result=true;
	cerr << "Checking stpvars" << endl;
	if(stpvars)
	{
		const char *olddriver=stp_get_driver(stpvars);
		cerr << "Checking olddriver" << endl;
		if(!olddriver)
			olddriver="None";
		cerr << "Checking driver" << endl;
		if(!driver)
			driver=DEFAULT_PRINTER_DRIVER;
		cerr << "Comparing drivers:" << olddriver << " against " << driver << endl;
		if(strcmp(driver,olddriver))
		{
			cerr << "SetDriver(): Setting driver to " << driver << endl;

			// Work around the non-defaulting of inactive settings...
			const stp_vars_t *defaults=stp_default_settings();
			stp_vars_copy(stpvars,defaults);

			stp_set_driver(stpvars,driver);
			output.SetString("Driver",driver);

			const stp_printer_t *printer=stp_get_printer(stpvars);
			cerr << "Checking printer" << endl;
			if(printer)
			{
				cerr << "Setting defaults" << endl;
				stp_set_printer_defaults(stpvars,printer);
			}
			else
			{
				cerr << "Unable to get printer - reverting to default driver" << endl;
				output.SetString("Driver",DEFAULT_PRINTER_DRIVER);
				stp_set_driver(stpvars,DEFAULT_PRINTER_DRIVER);
				cerr << "Checking printer again" << endl;
				if((printer=stp_get_printer(stpvars)))
					stp_set_printer_defaults(stpvars,printer);
				else
					cerr << "Still can't get printer!" << endl;
				result=false;
			}
		}
		stp_set_page_width(stpvars,pagewidth);
		stp_set_page_height(stpvars,pageheight);


		stp_parameter_t desc;
		stp_describe_parameter(stpvars,"PPDFile",&desc);
		cerr << "PPDFile parameter active? " << (desc.is_active ? "Yes" : "No") << endl;
//		FIXME!!!
		cerr << "Checking desc.is_active" << endl;
//		FIXME - Ugly hack to get around the PPDFile parameter being active
		if(desc.is_active)
//		if(0)
		{
			// FIXME - why is PPDFile parameter active for escp2-r300 driver?
			cerr << "Getting default PPD..." << endl;
			char *defppd=output.GetPPD();
			cerr << "Checking defppd" << endl;
			if(defppd)
			{
				cerr << "Setting PPDFile to " << defppd << endl;
				stp_set_file_parameter(stpvars,"PPDFile",defppd);
				free(defppd);
				
				// FIXME - does this break anything?

				cerr << "Checking ppdsizes_workaround_done" << endl;
				if(!ppdsizes_workaround_done)
				{
					stp_parameter_t desc2;
					stp_describe_parameter(stpvars,"PageSize",&desc2);
					cerr << "After setting PPD Default page size is now: " << desc2.deflt.str << endl;
					stp_set_string_parameter(stpvars,"PageSize",desc2.deflt.str);
					stp_parameter_description_destroy(&desc2);
					ppdsizes_workaround_done=true;
				}
			}
		}
		stp_parameter_description_destroy(&desc);
	}
	else
		cerr << "No stp vars!" << endl;
	return(result);
}


void GPrinterSettings::Validate()
{
	stputil_validate_parameters(stpvars);
	cerr << "After stputil_validate_parameters(): " << setlocale(LC_ALL,"") << endl;
}


void GPrinterSettings::Reset()
{
	stp_vars_destroy(stpvars);
	stpvars=stp_vars_create();
	// FIXME - a bit of a hack this - set the driver so we can compare against it.
	stp_set_driver(stpvars,"ps");
	cerr << "Created fresh stp_vars" << endl;
	initialised=false;
}
