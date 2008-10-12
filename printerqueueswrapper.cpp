#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "printerqueueswrapper.h"
#include "support/generaldialogs.h"

using namespace std;

static char *getfilename(void *userdata)
{
	return(File_Save_Dialog("Save printer spool file",NULL));
}


PrinterQueues::PrinterQueues()
{
	queues=pqinfo_create();
	queues->SetGetFilenameCallback(queues,getfilename,NULL);
}


PrinterQueues::~PrinterQueues()
{
	cerr << "Disposing of printer queues\n" << endl;
	if(queues)
		queues->Dispose(queues);
	cerr << "Done" << endl;
}


char *PrinterQueues::GetPrinterName(int idx)
{
	return(queues->GetPrinterName(queues,idx));
}


int PrinterQueues::GetPrinterCount()
{
	return(queues->GetPrinterCount(queues));
}


char *PrinterQueues::GetPrinterDriver(const char *printername)
{
	queues->SetPrinterQueue(queues,printername);
	return(queues->GetDriver(queues));
}


char *PrinterQueues::GetPrinterDriver()
{
	return(queues->GetDriver(queues));
}


struct pqinfo *PrinterQueues::GetPQInfo()
{
	return(queues);
}


const char *PrinterQueues::GetPrinterQueue()
{
	return(queues->GetPrinterQueue(queues));
}


void PrinterQueues::SetPrinterQueue(const char *queue)
{
	queues->SetPrinterQueue(queues,queue);
}


bool PrinterQueues::PrinterQueueExists(const char *queue)
{
	int qc=GetPrinterCount();
	for(int i=0;i<qc;++i)
	{
		char *tmp=GetPrinterName(i);
		if(strcmp(queue,tmp)==0)
		{
			free(tmp);
			return(true);
		}
		free(tmp);
	}
	return(false);
}


char *PrinterQueues::GetDriver()
{
	return(queues->GetDriver(queues));
}


char *PrinterQueues::GetPPD()
{
	cerr << "Current queue: " << GetPrinterQueue() << endl;
	char *ppd=queues->GetPPD(queues);
	if(ppd)
		cerr << "PPD: " << ppd << endl;
	else
		cerr << "No PPD found" << endl;
	return(queues->GetPPD(queues));
}


const char *PrinterQueues::GetCustomCommand()
{
#ifndef WIN32
	return(queues->GetCustomCommand(queues));
#else
	return(NULL);
#endif
}


void PrinterQueues::SetCustomCommand(const char *cmd)
{
#ifndef WIN32
	queues->SetCustomCommand(queues,cmd);
#endif
}


bool PrinterQueues::InitialiseJob()
{
	return(queues->InitialiseJob(queues)!=0);
}


void PrinterQueues::InitialisePage()
{
	queues->InitialisePage(queues);
}


void PrinterQueues::EndPage()
{
	queues->EndPage(queues);
}


void PrinterQueues::EndJob()
{
	queues->EndJob(queues);
}


void PrinterQueues::CancelJob()
{
	queues->CancelJob(queues);
}


int PrinterQueues::WriteData(const char *data,int bytecount)
{
	return(queues->WriteData(queues,data,bytecount));
}


