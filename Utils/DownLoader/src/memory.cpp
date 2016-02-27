#include "StdAfx.h"
#include "memory.h"

#ifdef DEBUG_MEMORY
MemNode * MemoryList = NULL;
CCriticalSection s_MemLock;
#endif

void MDump(void)
{
#ifdef DEBUG_MEMORY
	MemNode * m;

	m = MemoryList;

	if(m)
	{
		TRACE("Dumping Used Memory:\n");
		while(m)
		{
			TRACE("%d Bytes of %d %s: %s\n", m->Bytes, m->Count, m->TypeName, m->FileName);
			m = m->Next;
		}
	}
#endif
}

DWORD MUsage(void)
{
#ifdef DEBUG_MEMORY
	unsigned long Used;
	MemNode * m;

	m = MemoryList;

	Used = 0;
	while(m)
	{
		Used += m->Bytes;

		TRACE("%.8X: %s[%d]: %s\n", m->Ptr, m->TypeName, m->Count, m->FileName);

		m = m->Next;
	}

	TRACE("Memory Used: %s\n", Util::FormatBytes(Used));

	return Used;
#else
	return -1;
#endif
}