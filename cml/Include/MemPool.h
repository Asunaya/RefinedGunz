#ifndef _MemPool_h
#define _MemPool_h

//#pragma once

#include "Windows.h"
#include "Winbase.h"
#include "MDebug.h"
#include "assert.h"
#include <mutex>

#define InitMemPool(T)
#define UninitMemPool(T)
#define ReleaseMemPool(T)	CMemPool<T>::Release();

template< typename T >
class CMemPool
{
protected:

protected:
	static T*	m_list;
	T*			m_next;

	static std::mutex Mutex;

public:
	static void	Release();

public:
	static void* operator new( size_t size_ );
	static void  operator delete( void* deadObject_, size_t size_ );
public:
};

// new
template<typename T>
void* CMemPool<T>::operator new( size_t size_ )
{
	T* instance;

	std::lock_guard<std::mutex> lock(Mutex);

	if( m_list != NULL )
	{
		instance	= m_list;
		m_list	= m_list->m_next;
	}
	else
	{
		instance = (T*)::operator new(size_);
	}

#ifdef _DEBUG
	if(size_ != sizeof(*instance))
		assert(0);
#endif

	return instance;
}

// delete
template<typename T>
void CMemPool<T>::operator delete( void* deadObject_, size_t size_ )
{
	std::lock_guard<std::mutex> lock(Mutex);

	((T*)deadObject_)->m_next	= m_list;
	m_list	= (T*)deadObject_;
}

// Release
// 현재 할당되어 있지 않은 메모리 해제~~
template<typename T>
void CMemPool<T>::Release()
{
	// KeeperManager(PatchInterface)는 여러개의 클라이언트를 가질수 있음.
	//  2개이상의 클라이언트가 종료될때 이부분이 중복 호출이 되어, 
	//  유효하지 않은 m_csLock에 크리티컬섹션을 설정하여 예외가 발생함.
	//  리스트의 해더 주소가 '0'이 아닐경우만 수행하도록 수정. - by 추교성.
	if( NULL != m_list ) 
	{
		std::lock_guard<std::mutex> lock(Mutex);

		T* pInstace		= m_list;
		while( pInstace != NULL )
		{
			pInstace	= m_list->m_next;
			::operator delete( m_list );
			m_list	= pInstace;
		}
	}
}

template<typename T> std::mutex CMemPool<T>::Mutex;
template<typename T> T* CMemPool<T>::m_list;


////////////////////////////////////////////////////////////////////////////
// 쓰레드 고려 안하는~
////////////////////////////////////////////////////////////////////////////

template < typename T >
class CMemPoolSm
{
protected:
	static T*	m_list;
	T*			m_next;

public:
	static void* operator new( size_t size_ );
	static void  operator delete( void* deadObject_, size_t size_ );

	static void Release();

public:
	CMemPoolSm(void) {};
	~CMemPoolSm(void) {};
};


template<typename T>
void* CMemPoolSm<T>::operator new( size_t size_ )
{
	T* instance;

	if( m_list != NULL ) {
		instance = m_list;
		m_list = m_list->m_next;
//		mlog("메모리 재사용\n");
	} else {
		instance = (T*)::operator new(size_);
//		mlog("메모리 할당\n");
	}

#ifdef _DEBUG
	if(size_ != sizeof(*instance))
		assert(0);
#endif

	return instance;
}

template<typename T>
void CMemPoolSm<T>::operator delete( void* deadObject_, size_t size_ )
{
	((T*)deadObject_)->m_next = m_list;
	m_list	= (T*)deadObject_;
//	mlog("메모리 가상 삭제\n");
}



// 현재 할당되어 있지 않은 메모리 해제~~
template<typename T>
void CMemPoolSm<T>::Release()
{
	T* pInstace	= m_list;
	while( pInstace != NULL ) {
		pInstace = m_list->m_next;
		::operator delete( m_list );
		m_list = pInstace;
//		mlog("메모리 삭제\n");
	}
}

/////////////////////////////////////////////////////////////////////////

template<typename T> T* CMemPoolSm<T>::m_list;

#endif//_MemPool_h