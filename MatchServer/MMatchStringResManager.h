#ifndef _MMATCH_STRINGRES_MANAGER_H
#define _MMATCH_STRINGRES_MANAGER_H

#include "MBaseStringResManager.h"

class MMatchStringResManager : public MBaseStringResManager
{
public:
	MMatchStringResManager();
	virtual ~MMatchStringResManager();
	static void MakeInstance();
};


#endif