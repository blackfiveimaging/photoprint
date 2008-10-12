#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include <windows.h>


struct printernode
{
	void (*Dispose)(struct printernode *pn);
	struct printernode *next,*prev;
	struct pqprivate *head;
	char *name;
};


enum pqoutputmode
{
	PQMODE_PRINT,
	PQMODE_FILE
};


struct pqprivate
{
	void (*Dispose)(struct pqprivate *pp);
	char *currentqueue;
	struct printernode *first;
	HANDLE printer;
	enum pqoutputmode mode;
	char *(*getfilecallback)(void *userdata);
	void *userdata;
	FILE *outfile;
};


void pn_dispose(struct printernode *pn)
{
	if(pn->next)
		pn->next->prev=pn->prev;
	if(pn->prev)
		pn->prev->next=pn->next;
	else
		pn->head->first=pn->next;
	if(pn->name)
		free(pn->name);
	free(pn);
}


struct printernode *printernode_create(struct pqprivate *head,const char *name)
{
	struct printernode *pn=(struct printernode *)malloc(sizeof(struct printernode));
	if(pn)
	{
		struct printernode *p=head->first;
		pn->next=pn->prev=NULL;
		pn->Dispose=pn_dispose;
		pn->head=head;
		if(p)
		{
			while(p->next)
				p=p->next;
			p->next=pn;
			pn->prev=p;
		}
		else
			head->first=pn;

		pn->name=strdup(name);
	}
	return(pn);
}


static void pqp_dispose(struct pqprivate *pp)
{
	while(pp->first)
		pp->first->Dispose(pp->first);
	if(pp->currentqueue)
		free(pp->currentqueue);
	free(pp);
}


static void pqp_buildqueuelist(struct pqprivate *pp)
{
	DWORD dwSizeNeeded;
	DWORD dwNumItems;
	DWORD dwItem;
	LPPRINTER_INFO_2 lpInfo = NULL;

	EnumPrinters ( PRINTER_ENUM_LOCAL, NULL, 2, NULL, 0, &dwSizeNeeded, &dwNumItems );
	lpInfo = (LPPRINTER_INFO_2)HeapAlloc ( GetProcessHeap (), HEAP_ZERO_MEMORY, dwSizeNeeded );
	if(lpInfo)
	{
		if(EnumPrinters(PRINTER_ENUM_LOCAL,NULL,2,(LPBYTE)lpInfo,dwSizeNeeded,&dwSizeNeeded,&dwNumItems))
		{
			for ( dwItem = 0; dwItem < dwNumItems; dwItem++ )
			{
				printernode_create(pp,lpInfo[dwItem].pPrinterName);
			}
		}
	}
	HeapFree ( GetProcessHeap (), 0, lpInfo );

	printernode_create(pp,PRINTERQUEUE_SAVETOFILE);
}


static struct pqprivate *pqprivate_create()
{
	struct pqprivate *pp=(struct pqprivate *)malloc(sizeof(struct pqprivate));
	if(pp)
	{
		pp->Dispose=pqp_dispose;
		pp->first=NULL;
		pqp_buildqueuelist(pp);
	}
	return(pp);
}


/* pqinfo member functions */


static void dispose(struct pqinfo *pq)
{
	if(pq->priv)
		pq->priv->Dispose(pq->priv);
	free(pq);
}


static void setgetfilenamecallback(struct pqinfo *pq,char *(*func)(void *userdata),void *userdata)
{
	fprintf(stderr,"Setting filename callback.\n");
	pq->priv->getfilecallback=func;
	pq->priv->userdata=userdata;
}


static int getprintercount(struct pqinfo *pq)
{
	int count=0;
	struct printernode *p=pq->priv->first;
	while(p)
	{
		++count;
		p=p->next;
	}
	return(count);
}


static char *getprintername(struct pqinfo *pq,int index)
{
	struct printernode *p=pq->priv->first;
	while(--index>=0 && p)
	{
		p=p->next;
	}
	if(p)
		return(strdup(p->name));
	return(NULL);
}


static void setprinterqueue(struct pqinfo *pq,const char *queue)
{
	if(pq->priv->currentqueue)
		free(pq->priv->currentqueue);
	if(queue)
		pq->priv->currentqueue=strdup(queue);
	else
		pq->priv->currentqueue=NULL;
}


static const char *getprinterqueue(struct pqinfo *pq)
{
	return(pq->priv->currentqueue);
}


static char *getdriver(struct pqinfo *pq)
{
	/* FIXME - is there any way we can guess a suitable driver from the Win32 queue name? */
	return(NULL);
}


static char *getppd(struct pqinfo *pq)
{
	return(NULL);
}


static int initialisejob(struct pqinfo *pq)
{
	fprintf(stderr,"In initialisejob() - checking mode\n");
	if(strcmp(pq->priv->currentqueue,PRINTERQUEUE_SAVETOFILE)==0)
	{
		char *fn=NULL;

		pq->priv->mode=PQMODE_FILE;

		fprintf(stderr,"Getting filename...\n");

		if(pq->priv->getfilecallback)
			fn=pq->priv->getfilecallback(pq->priv->userdata);

		pq->priv->outfile=NULL;
		if(fn)
			pq->priv->outfile=fopen(fn,"wb");
		free(fn);

		if(!pq->priv->outfile)
			return(0);
		return(1);
	}
	else
	{
		static DOC_INFO_1 mydi={"Gutenprint output",NULL,NULL};
		BOOL result;

		fprintf(stderr,"Getting printer...\n");

		result=OpenPrinter(pq->priv->currentqueue,&pq->priv->printer,NULL);
		if(result)
			StartDocPrinter(pq->priv->printer,1,(LPBYTE)&mydi);
		pq->priv->mode=PQMODE_PRINT;
		return(result);
	}
}


static void initialisepage(struct pqinfo *pq)
{
	if(pq->priv->mode==PQMODE_PRINT)
	{
		if(pq->priv->printer)
			StartPagePrinter(pq->priv->printer);
	}
}


static void endpage(struct pqinfo *pq)
{
	if(pq->priv->mode==PQMODE_PRINT)
	{
		if(pq->priv->printer)
			EndPagePrinter(pq->priv->printer);
	}
}


static void endjob(struct pqinfo *pq)
{
	if(pq->priv->mode==PQMODE_PRINT)
	{
		if(pq->priv->printer)
		{
			EndDocPrinter(pq->priv->printer);
			ClosePrinter(pq->priv->printer);
		}
		pq->priv->printer=NULL;
	}
	else
	{
		if(pq->priv->outfile)
			fclose(pq->priv->outfile);
		pq->priv->outfile=NULL;
	}
}


static int writedata(struct pqinfo *pq,const char *data,int bytecount)
{
	if(pq->priv->mode==PQMODE_PRINT)
	{
		long written=0;
		WritePrinter(pq->priv->printer,(char *)data,bytecount,&written);
		return((written!=0));
	}
	else
	{
		long written=fwrite((char *)data,1,bytecount,pq->priv->outfile);
		return((written!=0));
	}
}


static void canceljob(struct pqinfo *pq)
{

}


struct pqinfo *pqinfo_create()
{
	struct pqinfo *pq=(struct pqinfo *)malloc(sizeof(struct pqinfo));
	if(pq)
	{
		pq->Dispose=dispose;
		pq->GetPrinterCount=getprintercount;
		pq->GetPrinterName=getprintername;
		pq->SetPrinterQueue=setprinterqueue;
		pq->GetPrinterQueue=getprinterqueue;
		pq->GetDriver=getdriver;
		pq->GetPPD=getppd;
		pq->InitialiseJob=initialisejob;
		pq->InitialisePage=initialisepage;
		pq->EndPage=endpage;
		pq->EndJob=endjob;
		pq->CancelJob=canceljob;
		pq->WriteData=writedata;
		pq->SetGetFilenameCallback=setgetfilenamecallback;

		if(!(pq->priv=pqprivate_create()))
		{
			pq->Dispose(pq);
			return(NULL);
		}
	}
	return(pq);
}
