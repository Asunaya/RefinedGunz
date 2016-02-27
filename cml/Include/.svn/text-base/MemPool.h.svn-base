#ifndef _MemPool_h
#define _MemPool_h

//#pragma once

#include "Windows.h"
#include "Winbase.h"
#include "MDebug.h"
#include "assert.h"

//////////////////////////////////////////////////////////////////////////
//
//	MemoryPool
//	* new, delete operator overload
//	* 메모리 할당관리
//		- 객체별로 하나의 MemoryPool 생성
//		- delete : 실제 delete하지 않고 memory pool에 저장
//		- new : 현재 비어 있는 메모리가 있으면 할당 else 실제 객체의 new를 호출
//
//	* 사용법
//		- CMemPool을 사용하는 객체는 CMemPool을 상속받는다
//		- 사용하기 전 InitMemPool을 호출한다
//		- 사용후 더이상 사용하지 않을때 UninitMemPool을 호출한다
//		- 언제라도 Free Slot에 Hold되어 있는 메모리를 해제하고 싶으면 ReleaseMemPool을 호출한다
//
//	* 예
//
//	class CTest:public CMemPool<CTest>
//	{
//	public:
//		CTest();
//		virtual ~CTest();
//		...
//	};		
//
//	InitMemPool(CTest);
//		...
//	CTest* p = new CTest;
//		...
//	delete(p);
//		...
//	ReleaseMemPool(CTest);
//		...
//	UninitMemPool(CTest);
//		...
//
//	* 참고 
//		- new 한 뒤 delete하지 않은 메모리에 대한 책임을 지지 않는다(메모리 누수)
//		- 메모리 강간에 대한 처리를 지원하지 않는다
//		- new[]와 delete[]는 적용되지 않는다
//		- MultiThread에도 적용되도록 만들었으나 테스트는 해보지 않았음..
//
//////////////////////////////////////////////////////////////////////////

#define InitMemPool(T)		CMemPool<T>::_InitCS()
#define UninitMemPool(T)	CMemPool<T>::_DeleteCS();
#define ReleaseMemPool(T)	CMemPool<T>::Release();

template< typename T >
class CMemPool
{
protected:

protected:
	static T*	m_list;
	T*			m_next;

	// Multi Thread에서 메모리를 보호하기 위해
	static CRITICAL_SECTION m_csLock;

public:
	static void	_InitCS()
	{
		InitializeCriticalSection( &m_csLock );
	}

	static void	_DeleteCS()
	{
		DeleteCriticalSection( &m_csLock );
	}
	static void	Release();

public:
	static void* operator new( size_t size_ );
	static void  operator delete( void* deadObject_, size_t size_ );
public:
	CMemPool(void)	{};
	~CMemPool(void)	{};
};

// new
template<typename T>
void* CMemPool<T>::operator new( size_t size_ )
{
	T* instance;
	EnterCriticalSection( &m_csLock );

	if( m_list != NULL )
	{
		instance	= m_list;
		m_list	= m_list->m_next;
	}
	else
	{
		instance = (T*)::operator new(size_);
	}

	LeaveCriticalSection( &m_csLock );

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
	EnterCriticalSection( &m_csLock );

	((T*)deadObject_)->m_next	= m_list;
	m_list	= (T*)deadObject_;

	LeaveCriticalSection( &m_csLock );
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
		EnterCriticalSection( &m_csLock );

		T* pInstace		= m_list;
		while( pInstace != NULL )
		{
			pInstace	= m_list->m_next;
			::operator delete( m_list );
			m_list	= pInstace;
		}

		LeaveCriticalSection( &m_csLock );
	}
}

template<typename T> CRITICAL_SECTION CMemPool<T>::m_csLock;
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