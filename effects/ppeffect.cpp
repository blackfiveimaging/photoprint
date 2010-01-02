#include <iostream>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "../support/debug.h"

#include "ppeffect.h"

using namespace std;

PPEffectHeader::PPEffectHeader() : RWMutex(), firsteffect(NULL)
{
}


PPEffectHeader::PPEffectHeader(PPEffectHeader &pp) : RWMutex(), firsteffect(NULL)
{
	Debug[TRACE] << "In PPEffectHeader's Copy Constructor" << endl;
	pp.ObtainMutexShared();
	PPEffect *e=pp.GetFirstEffect();
	while(e)
	{
		e->Clone(*this);
		e=e->Next();
	}
	pp.ReleaseMutex();
}


PPEffectHeader::~PPEffectHeader()
{
	Debug[TRACE] << "In PPEffectHeader's Destructor" << endl;
	ObtainMutex();
	while(firsteffect)
		delete firsteffect;
	ReleaseMutex();
}


ImageSource *PPEffectHeader::ApplyEffects(ImageSource *source,enum PPEFFECT_STAGE stage)
{
	Debug[TRACE] << "ApplyEffects - Obtain" << endl;
	ObtainMutexShared();
	PPEffect *effect=GetFirstEffect(stage);
	while(effect)
	{
		source=effect->Apply(source);
		effect=effect->Next(stage);	
	}
	Debug[TRACE] << "ApplyEffects - Release" << endl;
	ReleaseMutex();
	return(source);
}


int PPEffectHeader::EffectCount(enum PPEFFECT_STAGE stage)
{
	Debug[TRACE] << "EffectCount - obtain" << endl;
	ObtainMutexShared();
	PPEffect *effect=GetFirstEffect(stage);
	int count=0;
	while(effect)
	{
		count++;
		effect=effect->Next(stage);
	}
	Debug[TRACE] << "EffectCount - Release" << endl;
	ReleaseMutex();
	return(count);
}


PPEffect *PPEffectHeader::GetFirstEffect(enum PPEFFECT_STAGE stage)
{
	Debug[TRACE] << "GetFirstEffect - Obtain" << endl;
//	if(!AttemptMutex())
//		Debug[TRACE] << "GetFirstEffect - deadlock..." << endl;
	ObtainMutexShared();
	PPEffect *result=NULL;
	PPEffect *tmp=firsteffect;
	while(tmp)
	{
		if(tmp->stage & stage)
		{
			result=tmp;
			tmp=NULL;
		}
		else
			tmp=tmp->Next(stage);
	}
	Debug[TRACE] << "GetFirstEffect - Release" << endl;
	ReleaseMutex();
	return(result);
}


PPEffect *PPEffectHeader::Find(const char *id)
{
	if(!id)
		throw "PPEffectHeader::Find: No ID provided";
	Debug[TRACE] << "Find - Obtain" << endl;
//	if(!AttemptMutex())
//		Debug[TRACE] << "Find - deadlock..." << endl;
	ObtainMutexShared();
	PPEffect *result=NULL;
	PPEffect *tmp=firsteffect;
	while(tmp)
	{
		if(strcmp(tmp->GetID(),id)==0)
		{
			
			result=tmp;
			tmp=false;
		}
		else
			tmp=tmp->Next(PPEFFECT_DONTCARE);
	}
	Debug[TRACE] << "Find - Releasing" << endl;
	ReleaseMutex();
	return(result);
}


void PPEffectHeader::ObtainMutex()
{
	// We over-ride the ObtainMutex virtual function here

	RWMutex::ObtainMutex();
}


// We have a node list that works like this:
// Header  <->  Node 1  <->  Node 2  <->  Node 3...
// Each node contains a priority field, so we can slot newly inserted nodes
// into the correct point.

PPEffect::PPEffect(PPEffectHeader &header,int priority,enum PPEFFECT_STAGE stage)
	: priority(priority), stage(stage), header(header),prev(NULL),next(NULL)
{
	Debug[TRACE] << "Effect constructor - obtain" << endl;
	header.ObtainMutex();	// Exclusive lock needed
	PPEffect *node;
	if((node=header.firsteffect))
	{
		// Header  <->  node  <->  ...

		bool done=false;
		do
		{
			if(priority>node->priority)
			{
				// We've found a node with lower priority than this new one
				// So insert beforehand.

				next=node;

				// Header  <->  ...  <//>  this  </->  node... 

				if(node->prev==NULL)
				{
					// If the node being examined has no previous node, we're at the
					// head of the list, so we mark it in the header.
					node->prev=this;
					header.firsteffect=this;
					prev=NULL;
					done=true;
					// Header  <->  this  <->  node...
				}
				else
				{
					// Otherwise, slot this node between the prev node and the one
					// before that.
					node->prev->next=this;
					// Header  <->  ...  </->  this  </->  node  <->  ...
					prev=node->prev;
					// Header  <->  ...  <->  this  </->  node  <->  ...
					node->prev=this;
					// Header  <->  ...  <->  this  <->  node  <->  ...
					done=true;
				}
			}
			// Next node.
			if(node)
			{
				if(node->next)
					node=node->next;
				else if (!done)
				{
					// We've reached the end of the list, and not found any nodes of
					// lower priority - so we tack this node on the end of the list.
					node->next=this;
					prev=node;
					done=true;
				}
			}
		} while(!done);
	}
	else
	{
		// Situation is:
		// Header  <->  <NULL>  -  easy to deal with.
		header.firsteffect=this;
	}
	Debug[TRACE] << "effect constructor - release" << endl;
	header.ReleaseMutex();
}


PPEffect::~PPEffect()
{
	Debug[TRACE] << "Effect destructor - Obtain" << endl;
	header.ObtainMutex();	// Exclusive lock
	if(prev)
		prev->next=next;
	else
		header.firsteffect=next;
	if(next)
		next->prev=prev;
	Debug[TRACE] << "Effect destructor - Release" << endl;
	header.ReleaseMutex();
}


PPEffect *PPEffect::Next(enum PPEFFECT_STAGE stage)
{
	PPEffect *effect=next;
	while(effect)
	{
		if(effect->stage & stage)
			return(effect);
		effect=effect->next;
	}
	return(NULL);
}

