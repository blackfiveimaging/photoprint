#ifndef PRINTERQUEUES_H
#define PRINTERQUEUES_H

#ifdef __cplusplus
        extern "C" {
#endif

#define PRINTERQUEUE_CUSTOMCOMMAND "<Use custom print command>"
#define PRINTERQUEUE_SAVETOFILE "<Save to file>"

struct pqprivate;

struct pqinfo
{
	void (*Dispose)(struct pqinfo *pq);

	/***** For querying the available printer queues *****/

	int (*GetPrinterCount)(struct pqinfo *pq);
	char *(*GetPrinterName)(struct pqinfo *pq,int index);

	/* Get and set the current printer queue... */
	const char *(*GetPrinterQueue)(struct pqinfo *pq);
	void (*SetPrinterQueue)(struct pqinfo *pq,const char *queue);

	/* Returns the gutenprint driver associated with
	   the queue, or NULL if unknown.  */

	char *(*GetDriver)(struct pqinfo *pq);
	
	/* Returns the PPD associated with the the queue,
	   or NULL if unknown or not applicable. */

	char *(*GetPPD)(struct pqinfo *pq);


	/***** Output Options *****/

#ifndef WIN32
	/* Custom command (Unix only)
	   If a custom command is set, then instead of piping data to the
	   queue's default command, it will be piped to the custom command. */

	const char *(*GetCustomCommand)(struct pqinfo *pq);
	void (*SetCustomCommand)(struct pqinfo *pq,const char *cmd);
#endif

	/* Saving to a file (Unix and Windows)
	   Redirects the print data to a file.
	   Uses a callback function to get the output filename at print-time.
       Use like this:

	   Define a function that returns a filename - must return a full path,
	   which will be free()d once the file is open:

       char *mygetfnfunc(void *userdata)
	   {
	     struct myuserdata *ud=(struct myuserdata *ud)userdata;
		 [do user interface stuff here]
	     return(filename);
	   }
	   
	   pqinfo->SetGetFilenameCallback(pqinfo,mygetfnfunc,userdata);
	   
	*/

	void (*SetGetFilenameCallback)(struct pqinfo *pq,char *(*getfilename)(void *userdata),void *userdata);
	
	/***** Job Handling *****/

	int (*InitialiseJob)(struct pqinfo *pq);
	void (*InitialisePage)(struct pqinfo *pq);
	void (*EndPage)(struct pqinfo *pq);
	void (*EndJob)(struct pqinfo *pq);
	void (*CancelJob)(struct pqinfo *pq);

	/* Function to write data to the printer queue */
	int (*WriteData)(struct pqinfo *pq,const char *data,int bytecount);

	struct pqprivate *priv;
};

struct pqinfo *pqinfo_create();

#ifdef __cplusplus
    }
#endif

#endif
