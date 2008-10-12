#include <cstdlib>
#include <cstring>
#include <iostream>

#include "rangeparser.h"

using namespace std;


RangeParser::RangeParser(const char *range,int max)
	: max(max), range(NULL), ptr(NULL), prev(0), target(0)
{
	if(range)
		this->range=strdup(range);
	else
		this->range=strdup("");
	ptr=this->range;
	while(Next());
}


RangeParser::~RangeParser()
{
	if(range)
		free(range);
}


int RangeParser::Next()
{
	// If we're in the middle of a range, continue through the range...
	if(target)
	{
		if(prev<target)
			++prev;
		else
			prev=target=0;
	}
	if(!target)
	{
		// Skip over whitespace and commas.
		while(*ptr==' ' || *ptr==',')
			++ptr;
	
		// Check for end of string...
		// If reached, then we set the point back to the beginning;
		// This allows the constructor to do a "dummy run" through the
		// range, to verify that the specification is valid.
		if(!*ptr)
		{
			ptr=range;
			prev=target=0;
			return(0);
		}
		
		// Find end of current number...
		char *p2=ptr;
		if(*p2<'0' || *p2>'9')
			throw "Bad range specified";
		while(*p2>='0' && *p2<='9')
			++p2;
		// Replace character after number with 0, temporarily...	
		char tmp=*p2;
		prev=atoi(ptr);
		*p2=tmp;
	
		while(*p2!=0 && *p2==' ')
			++p2;
	
		ptr=p2;
	
		// Is a range specified?
		if(*ptr=='-')
		{
			++ptr;
			// Skip over whitespace
			while(*ptr==' ' || *ptr==',')
				++ptr;
	
			p2=ptr;
	
			if(!*ptr)
				target=max;
			else
			{
				if(*p2<'0' || *p2>'9')
					throw "Bad range specified";
	
				while(*p2>='0' && *p2<='9')
					++p2;
				// Replace character after number with 0, temporarily...	
				char tmp=*p2;
				target=atoi(ptr);
				*p2=tmp;
			}
			ptr=p2;
		}
	}
	if(max>0 && prev>max)
		throw "Bad range specified";
	return(prev);
}
