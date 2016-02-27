#pragma once

#define MAlloc(Type, Count) Mem_Alloc<Type>(#Type, Count, __LINE__, __FILE__)

#ifdef DEBUG_MEMORY
struct MemNode
{
	void * Ptr;
	char TypeName[64];
	char FileName[256];
	int Count;
	int Bytes;

	MemNode * Next;
};

extern MemNode * MemoryList;
extern CCriticalSection s_MemLock;
#endif

template <class Type>
__inline Type * Mem_Alloc(const char * TypeName, int Count, int Line, const char * File)
{
#ifdef DEBUG_MEMORY
	s_MemLock.Lock(INFINITE);

	Type * Result;

	Result = new Type [Count];

	MemNode * m;

	m = new MemNode;

	m->Ptr = Result;
	m->Count = Count;
	m->Bytes = sizeof(Type) * Count;
	strcpy(m->TypeName, TypeName);
	sprintf(m->FileName, "%s: Line %d", File, Line);
	m->Next = NULL;

	if(!MemoryList)
	{
		MemoryList = m;
	}else{
		m->Next = MemoryList;
		MemoryList = m;
	}

	s_MemLock.Unlock();

	return Result;
#else
	return new Type [Count];
#endif
}

template <class Type>
__inline void MFree(Type *& Ptr)
{
#ifdef DEBUG_MEMORY
	s_MemLock.Lock(INFINITE);

	MemNode * p;
	MemNode * m;
	bool Found = false;

	m = MemoryList;
	p = NULL;
	while(m)
	{
		if(Ptr == m->Ptr)
		{
			if(p == NULL)
			{
				MemoryList = m->Next;
			}else{
				p->Next = m->Next;
			}

			delete m;
			Found = true;
			break;
		}
		
		p = m;
		m = m->Next;
	}

	if(Found == false)
	{
		TRACE("Warning: Attempt to Delete Invalid Pointer (%.8X)\n", (DWORD_PTR)Ptr);
		ASSERT(FALSE);

		s_MemLock.Unlock();

		return;
	}else{
		delete [] Ptr;
	}

	Ptr = NULL;

	s_MemLock.Unlock();
#else

	delete [] Ptr;
	Ptr = NULL;

#endif
}

void MDump(void);
DWORD MUsage(void);
