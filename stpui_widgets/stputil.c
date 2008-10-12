#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stputil.h"

void stputil_validate_parameters(stp_vars_t *v)
{
	stp_parameter_list_t params = stp_get_parameter_list(v);
	
	int count = stp_parameter_list_count(params);
	int i;
	for (i = 0; i < count; i++)
	{
		const stp_parameter_t *p = stp_parameter_list_param(params, i);
		stp_parameter_t desc;
		stp_describe_parameter(v,p->name,&desc);

		if(desc.is_active && desc.p_level<=STP_PARAMETER_LEVEL_ADVANCED4)
		{
			switch(desc.p_type)
			{
				case STP_PARAMETER_TYPE_STRING_LIST:
					{
						int idx=-1;
						if(stp_check_string_parameter(v,desc.name,STP_PARAMETER_DEFAULTED))
						{
							const char *val=stp_get_string_parameter(v,desc.name);
							stp_string_list_t *strlist=desc.bounds.str;
							if(strlist)
							{
								int j;
								int strcount=stp_string_list_count(strlist);
								for(j=0;j<strcount;++j)
								{
									stp_param_string_t *p=stp_string_list_param(strlist,j);
									if(strcmp(p->name,val)==0)
									{
										idx=j;
										j=strcount;
									}
								}
							}
						}
						if(idx<0 && desc.is_mandatory)
						{
							fprintf(stderr,"Setting %s to default value %s\n",desc.name,desc.deflt.str);
							stp_set_string_parameter(v,desc.name,desc.deflt.str);
						}
					}
					break;

				case STP_PARAMETER_TYPE_INT:
					{
						int setdefault=1;
						if(stp_check_string_parameter(v,desc.name,STP_PARAMETER_DEFAULTED))
						{
							int val=stp_get_int_parameter(v,desc.name);
							if(val>=desc.bounds.integer.lower && val<=desc.bounds.integer.upper)
							{
								setdefault=0;
							}
						}
						if(setdefault && desc.is_mandatory)
							stp_set_int_parameter(v,desc.name,desc.deflt.integer);
					}
					break;

				case STP_PARAMETER_TYPE_BOOLEAN:
					if(desc.is_mandatory && !stp_check_boolean_parameter(v,desc.name,STP_PARAMETER_DEFAULTED))
						stp_set_boolean_parameter(v,desc.name,desc.deflt.boolean);
					break;

				case STP_PARAMETER_TYPE_DOUBLE:
					{
						int setdefault=1;
						if(stp_check_string_parameter(v,desc.name,STP_PARAMETER_DEFAULTED))
						{
							int val=stp_get_int_parameter(v,desc.name);
							if(val>=desc.bounds.integer.lower && val<=desc.bounds.integer.upper)
							{
								setdefault=0;
							}
						}
						if(setdefault && desc.is_mandatory)
							stp_set_int_parameter(v,desc.name,desc.deflt.integer);
					}
					break;

				case STP_PARAMETER_TYPE_DIMENSION:
					{
						int setdefault=1;
						if(stp_check_dimension_parameter(v,desc.name,STP_PARAMETER_DEFAULTED))
						{
							int val=stp_get_dimension_parameter(v,desc.name);
							if(val>=desc.bounds.dimension.lower && val<=desc.bounds.dimension.upper)
							{
								setdefault=0;
							}
						}
						if(setdefault && desc.is_mandatory)
							stp_set_dimension_parameter(v,desc.name,desc.deflt.dimension);
					}
					break;

				default:
					break;					
			}
		}
	}
	stp_parameter_list_destroy(params);
}

