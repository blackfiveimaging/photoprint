#ifndef CMTRANSFORMWORKER_H
#define CMTRANSFORMWORKER_H

//////////////  Conversion Worker Thread - ///////////////
// A subclass of the generic worker thread which has a
// thread-specific TransformFactory, to dodge LCMS's thread safety issues.


class CMTransformWorker : public Worker
{
	public:
	CMTransformWorker(JobQueue &queue,ProfileManager &pm) : Worker(queue), profilemanager(pm)
	{
		factory=profilemanager.GetTransformFactory();
	}
	virtual ~CMTransformWorker()
	{
		WaitCompletion();
		delete factory;
	}
	ProfileManager &profilemanager;
	CMTransformFactory *factory;
};

#endif

