#include <iostream>

#include "gutenprint/gutenprint.h"
using namespace std;


int main(int argc, char **argv)
{
	stp_init();


	stp_vars_t *stpvars;
	stpvars=stp_vars_create();

	const stp_vars_t *defaults=stp_default_settings();
	stp_vars_copy(stpvars,defaults);

	stp_set_driver(stpvars,"ps2");

	const stp_printer_t *printer=stp_get_printer(stpvars);
	if(printer)
	{
		stp_set_printer_defaults(stpvars,printer);
	}


	stp_parameter_list_t params = stp_get_parameter_list(stpvars);
	int count = stp_parameter_list_count(params);

	for (int i = 0; i < count; i++)
	{
		const stp_parameter_t *p = stp_parameter_list_param(params, i);
		stp_parameter_t desc;
		stp_describe_parameter(stpvars,p->name,&desc);
		switch(desc.p_type)
		{
			case STP_PARAMETER_TYPE_FILE:
				cerr << "Parameter: " << desc.name << endl;
				cerr << "  Active?: " << (desc.is_active!=0) << endl;
				break;
			default:
				break;
		}
		stp_parameter_description_destroy(&desc);
	}
	stp_parameter_list_destroy(params);

	if(stpvars)
		stp_vars_destroy(stpvars);
}

