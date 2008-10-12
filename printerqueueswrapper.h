#ifndef PRINTERQUEUES_WRAPPER_H
#define PRINTERQUEUES_WRAPPER_H

#include "stp_support/printerqueues.h"

class PrinterQueues
{
	public:
	PrinterQueues();
	~PrinterQueues();
	int GetPrinterCount();
	char *GetPrinterName(int idx);
	char *GetPrinterDriver(const char *printername);
	char *GetPrinterDriver();

	const char *GetPrinterQueue();
	void SetPrinterQueue(const char *queue);
	bool PrinterQueueExists(const char *queue);

	char *GetDriver();
	char *GetPPD();

	const char *GetCustomCommand();
	void SetCustomCommand(const char *cmd);

	bool InitialiseJob();
	void InitialisePage();
	void EndPage();
	void EndJob();
	void CancelJob();

	int WriteData(const char *data,int bytecount);

	struct pqinfo *GetPQInfo();

	protected:
	struct pqinfo *queues;
};

#endif
