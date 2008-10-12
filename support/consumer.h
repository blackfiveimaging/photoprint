/*
 * consumer.h - classes for writing to disk or piping raw binary data.
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef CONSUMER_H
#define CONSUMER_H
#include <iostream>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

using namespace std;

class Consumer
{
	public:
	virtual ~Consumer()
	{
	}
	virtual bool Write(const char *buffer, int length)=0;
	virtual void Cancel()=0;
};


class Consumer_File : public Consumer
{
	public:
	Consumer_File(const char *filename);
	~Consumer_File();
	bool Write(const char *buffer,int length);
	void Cancel();
	private:
	FILE *file;
};


class Consumer_Pipe : public Consumer
{
	public:
	Consumer_Pipe(const char *command);
	~Consumer_Pipe();
	bool Write(const char *buffer,int length);
	void Cancel();
	private:
	static void sighandler(int sig);
#ifdef WIN32
	FILE *pfile;
#else
	int pipefd[2];
	int childpid;
#endif
	bool canceled;
	static bool aborted;
};

#endif
