#ifndef PRINTOUTPUT_H
#define PRINTOUTPUT_H

#include "support/configdb.h"
#include "support/consumer.h"
#include "printerqueueswrapper.h"

#define DEFAULT_PRINTER_DRIVER "escp2-600"

class PrintOutput : public ConfigDB, public PrinterQueues
{
	public:
	PrintOutput(ConfigFile *inif,const char *section);
	Consumer *GetConsumer();
	void DBToQueues();
	void QueuesToDB();
	private:
	static ConfigTemplate Template[];
	char *str2;
};

#endif
