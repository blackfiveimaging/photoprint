#include <stdio.h>
#include <string.h>

#include "printerqueues.h"

static char *getfilename(void *userdata)
{
	return(strdup("/tmp/testprintcapture"));
}


int main(int argc,char **argv)
{
	struct pqinfo *pq;

	if((pq=pqinfo_create()))
	{
		char *message="This is a quick test of printing...\n";
		int pcount=pq->GetPrinterCount(pq);
		int i;
		printf("%d printer queues available\n",pcount);
		for(i=0;i<pcount;++i)
		{
			const char *pn=pq->GetPrinterName(pq,i);
			const char *driver;
			printf("Printer: %s\n",pn);
			pq->SetPrinterQueue(pq,pn);
			driver=pq->GetDriver(pq);
			if(driver)
				printf("(using driver: %s)\n",driver);
			else
				printf("(driver unknown)\n");
		}

#ifdef WIN32
		pq->SetPrinterQueue(pq,"Epson Stylus Photo R300");
		pq->SetGetFilenameCallback(pq,getfilename,NULL);
#else
		pq->SetPrinterQueue(pq,PRINTERQUEUE_CUSTOMCOMMAND);
		pq->SetCustomCommand(pq,"cat >test.prn");
#endif
		pq->InitialiseJob(pq);
		pq->InitialisePage(pq);
		pq->WriteData(pq,message,strlen(message));
		pq->EndPage(pq);
		pq->EndJob(pq);

		pq->Dispose(pq);
	}

	return(0);
}
