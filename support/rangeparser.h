#ifndef RANGEPARSER_H
#define RANGEPARSER

// A class to handle parsing string-based page ranges, of the form
// "1,3,5, 7-9, 15"
// Pass in the page description to the constructor, along with an optional
// maximum allowed page number.

// Each successive call to Next() will return the next page number in the sequence,
// finally returning 0 to indicate the end of the sequence.
// Further calls to Next() will restart the sequence at the beginning...

class RangeParser
{
	public:
	RangeParser(const char *range,int max=0);
	~RangeParser();
	int Next();
	private:
	int max;
	char *range;
	char *ptr;
	int prev;
	int target;
};


#endif
