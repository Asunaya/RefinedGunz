#pragma once

#include "math.h"

/**		List template class
*		CCriticalSection을 이용한 Thread Safe
*
*		@author soyokaze
*		@date   2005-11-02
*/

template <class Type>
class List  
{
protected:

	struct Node
	{
		Type m_Data;
		Node * m_Next;
		Node * m_Prev;
	};

public:
	List(void)
	{
		m_Head = 0;
		m_Tail = 0;
		m_Count = 0;

		m_Last = 0;
		m_LastIndex = 0;
	}

	List(List & l)
	{
		*this = l;
	}

	virtual ~List(void)
	{
		while(!IsEmpty())
		{
			Delete(0);
		}
	}

	List & operator = (List & l)
	{
		DWORD i;

		DeleteAll();

		for(i = 0; i < l.m_Count; i++)
		{
			Insert(i, l[i]);
		}

		return *this;
	}

	void Insert(DWORD Index, const Type & Data)
	{
		Node * n;
		Node * Curr = m_Head;
		DWORD i;

		m_Lock.Lock(INFINITE);

		m_Last = 0;

		ASSERT(Index <= m_Count);

		n = NewNode();
		n->m_Data = Data;

		if(m_Head == 0)
		{
			m_Head = n;
			m_Tail = n;

			m_Count++;
			m_Lock.Unlock();
			return;
		}

		// Add to the Head
		if(Index == 0)
		{
			m_Head->m_Prev = n;
			n->m_Next = m_Head;
			m_Head = n;

			m_Count++;
			m_Lock.Unlock();
			return;
		}

		// Add to the Tail
		if(Index == m_Count)
		{
			m_Tail->m_Next = n;
			n->m_Prev = m_Tail;
			m_Tail = n;

			m_Count++;
			m_Lock.Unlock();
			return;
		}

		// Add in the Middle
		for(i = 0; i < Index; i++)
		{
			Curr = Curr->m_Next;
		}

		n->m_Prev = Curr->m_Prev;
		n->m_Next = Curr;
		Curr->m_Prev->m_Next = n;
		Curr->m_Prev = n;

		m_Count++;

		m_Lock.Unlock();
	}

	void InsertSorted(const Type & Data, int (*SortCallBack)(Type &, Type &))
	{
		Node *Curr = m_Head;
		DWORD i = 0;

		if(Curr == 0)
		{
			Insert(0, Data);
			return;
		}

		while(Curr != 0)
		{
			if(SortCallBack(Data, Curr->m_Data) > 0)
			{
				Insert(i, Data);
				return;
			}
			i++;
			Curr = Curr->m_Next;
		}

		Insert(i, Data);
	}

	void InsertSorted(const Type & Data, int (*SortCallBack)(Type, Type))
	{
		Node *Curr = m_Head;
		DWORD i = 0;

		if(Curr == 0)
		{
			Insert(0, Data);
			return;
		}

		while(Curr != 0)
		{
			if(SortCallBack(Data, Curr->m_Data) > 0)
			{
				Insert(i, Data);
				return;
			}
			i++;
			Curr = Curr->m_Next;
		}

		Insert(i, Data);
	}

	void DeleteAll(void)
	{
		while(!IsEmpty())
		{
			Delete(0);
		}
	}

	void Delete(DWORD Index)
	{
		Node * n;
		Node * Curr = m_Head;
		DWORD i;

		m_Lock.Lock(INFINITE);
		m_Last = 0;

		ASSERT(Index < m_Count);

		// Delete Head Element
		if(Index == 0)
		{
			n = m_Head;
			m_Head = m_Head->m_Next;

			if(m_Head != 0)
			{
				m_Head->m_Prev = 0;
			}

			MFree(n);

			m_Count--;

			m_Lock.Unlock();
			return;
		}

		// Delete Tail Element
		if(Index == m_Count - 1)
		{
			n = m_Tail;
			m_Tail = m_Tail->m_Prev;
			m_Tail->m_Next = 0;

			MFree(n);

			m_Count--;
			m_Lock.Unlock();
			return;
		}

		// Delete Middle Element
		for(i = 0; i < Index; i++)
		{
			Curr = Curr->m_Next;
		}

		if(Curr->m_Next != 0) Curr->m_Next->m_Prev = Curr->m_Prev;
		if(Curr->m_Prev != 0) Curr->m_Prev->m_Next = Curr->m_Next;

		MFree(Curr);
		m_Count--;

		m_Lock.Unlock();
	}

	Type & GetAt(DWORD Index)
	{
		return (*this)[Index];
	}

	Type & operator [] (DWORD Index)
	{
		DWORD ShortestDistance;
		Node * Curr;
		DWORD i;

		ASSERT(Index < m_Count);

		ShortestDistance = Index;

		m_Lock.Lock(INFINITE);

		if(m_Count - Index < ShortestDistance) ShortestDistance = m_Count - Index;
		if(m_Last != 0)
		{
			if(Index > m_LastIndex)
			{
				if(Index - m_LastIndex < ShortestDistance) ShortestDistance = Index - m_LastIndex;
			}else{
				if(m_LastIndex - Index < ShortestDistance) ShortestDistance = m_LastIndex - Index;
			}
		}

		// Start from Head
		if(ShortestDistance == Index)
		{
			Curr = m_Head;

			for(i = 0; i < Index; i++)
			{
				Curr = Curr->m_Next;
			}

			m_Last = Curr;
			m_LastIndex = Index;

			m_Lock.Unlock();
			return Curr->m_Data;
		}

		// Start from Tail
		if(ShortestDistance == m_Count - Index)
		{
			Curr = m_Tail;

			for(i = m_Count - 1; i > Index; i--)
			{
				Curr = Curr->m_Prev;
			}

			m_Last = Curr;
			m_LastIndex = Index;

			m_Lock.Unlock();
			return Curr->m_Data;
		}

		// Start from most Recent
		Curr = m_Last;

		if(Index > m_LastIndex)
		{
			for(i = m_LastIndex; i < Index; i++)
			{
				Curr = Curr->m_Next;
			}
		}else{
			for(i = m_LastIndex; i > Index; i--)
			{
				Curr = Curr->m_Prev;
			}
		}

		m_Last = Curr;
		m_LastIndex = Index;

		m_Lock.Unlock();
		return Curr->m_Data;
	}

	void Push(const Type & Data)
	{
		Insert(0, Data);
	}

	Type Pop(void)
	{
		Type Data;

		Data = GetAt(0);
		Delete(0);

		return Data;
	}

	void PushBack(const Type & Data)
	{
		Insert(m_Count, Data);
	}

	Type PopBack(void)
	{
		Type Data;

		Data = GetAt(m_Count - 1);
		Delete(m_Count - 1);

		return Data;
	}

	bool IsEmpty(void)
	{
		return (m_Count == 0) ? true : false;
	}

	const DWORD GetSize(void)
	{
		return m_Count;
	}

protected:

	Node * NewNode(void)
	{
		Node *n = MAlloc(Node, 1);
		
		n->m_Next = 0;
		n->m_Prev = 0;
		
		return n;
	}

	DWORD m_Count;
	Node * m_Head;
	Node * m_Tail;

	CCriticalSection m_Lock;

	Node * m_Last;
	DWORD m_LastIndex;
};