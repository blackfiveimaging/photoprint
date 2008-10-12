#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

#include "support/generaldialogs.h"

#include "printoutput.h"

#include "pp_printoutput.h"

class PODBHandler : public ConfigDBHandler
{
	public:
	PODBHandler(ConfigFile *inif,const char *section,PrintOutput *po)
		: ConfigDBHandler(inif,section,po), printoutput(po)
	{
	}
	~PODBHandler()
	{
	}
	void LeaveSection()
	{
		cerr << "*** Leaving PrintOutput section" << endl;
		printoutput->DBToQueues();
	}
	private:
	PrintOutput *printoutput;	
};

PrintOutput::PrintOutput(ConfigFile *inif,const char *section) : ConfigDB(Template), PrinterQueues()
{
	cerr << "In PrintOutput constructor..." << endl;
	new PODBHandler(inif,section,this);
	const char *defaultqueue=FindString("Queue");
	if(strlen(defaultqueue)==0 && GetPrinterCount()>0)
	{
		char *queue=GetPrinterName(0);
		
		if(queue)
		{
			SetString("Queue",queue);
			char *driver=GetPrinterDriver(queue);
			SetString("Driver",driver);
			free(driver);
			free(queue);
		}
		else
			SetString("Driver",DEFAULT_PRINTER_DRIVER);
	}
	cerr << "Done..." << endl;
}

class Consumer_Queue : public Consumer
{
	public:
	Consumer_Queue(PrinterQueues &pq,const char *queuename) : pq(pq)
	{
		pq.SetPrinterQueue(queuename);
		if(!pq.InitialiseJob())
			throw "Can't initialise!";
		pq.InitialisePage();
	}
	virtual ~Consumer_Queue()
	{
		pq.EndPage();
		pq.EndJob();
	}
	virtual bool Write(const char *buffer, int length)
	{
		return(pq.WriteData(buffer,length));
	}
	virtual void Cancel()
	{
		pq.CancelJob();
	}
	protected:
	PrinterQueues &pq;
};


Consumer *PrintOutput::GetConsumer()
{
	const char *str;
	if(strlen(str=FindString("Queue")))
	{
		try
		{
			Consumer *result=new Consumer_Queue(*this,str);	
			return(result);
		}
		catch(const char *err)
		{
			return(NULL);
		}
	}
	else
		return(NULL);
}


void PrintOutput::DBToQueues()
{
	const char *tmp=FindString("Queue");
	if(PrinterQueueExists(tmp))
		cerr << "Printer queue exists" << endl;
	else
	{
		cerr << "Warning - printer queue not found" << endl;
		pp_printoutput_queue_dialog(this);
		tmp=FindString("Queue");
	}
	SetPrinterQueue(tmp);
	tmp=FindString("Command");
	SetCustomCommand(tmp);
}


void PrintOutput::QueuesToDB()
{
	const char *tmp=GetPrinterQueue();
	SetString("Queue",tmp);
	tmp=GetCustomCommand();
	SetString("Command",tmp);
}


ConfigTemplate PrintOutput::Template[]=
{
	ConfigTemplate("Queue",""),	ConfigTemplate("Driver",""),
	ConfigTemplate("Command",""),
	ConfigTemplate()
};
