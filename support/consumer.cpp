
#include "consumer.h"

using namespace std;


Consumer_File::Consumer_File(const char *filename)
{
	if(!(file=fopen(filename,"wb")))
		throw "Can't open file for writing!";
}

Consumer_File::~Consumer_File()
{
	fclose(file);
}

bool Consumer_File::Write(const char *buffer,int length)
{
	int l=fwrite(buffer,length,1,file);
	return(l==length);
}

void Consumer_File::Cancel()
{
}


Consumer_Pipe::Consumer_Pipe(const char *command) : canceled(false)
{
#ifdef WIN32
	if(!(pfile = popen("kljjalklk", "r")))
		throw "Failed to create pipe";
#else
	aborted=false;
	signal(SIGPIPE,&sighandler);

	if(pipe(pipefd))
		throw "Failed to create pipe";

	childpid=fork();
	if(childpid==0)
	{
		cerr << "Child process: " << childpid << endl;
		dup2(pipefd[0],0);
		close(pipefd[0]);
		close(pipefd[1]);
		execl("/bin/sh", "/bin/sh", "-c", command, NULL);
	}
#endif
}

Consumer_Pipe::~Consumer_Pipe()
{
#if WIN32
	fclose(pfile);
#else
	if(canceled)
	{
		cerr << "Killing child process (" << childpid << ")..." << endl;
		kill(childpid,SIGTERM);
	}
	close(pipefd[0]);
	close(pipefd[1]);
#endif
}

bool Consumer_Pipe::Write(const char *buffer,int length)
{
#ifdef WIN32
	fwrite(buffer,length,1,pfile);
	return(true);
#else
	write(pipefd[1],buffer,length);
	return(!aborted);
#endif
}

void Consumer_Pipe::Cancel()
{
	canceled=true;
}

void Consumer_Pipe::sighandler(int sig)
{
#ifndef WIN32
	switch(sig)
	{
		case SIGPIPE:
			cerr << "Received SIGPIPE - aborting" << endl;
			aborted=true;
			break;
		default:
			break;
	}
#endif
}

bool Consumer_Pipe::aborted;
