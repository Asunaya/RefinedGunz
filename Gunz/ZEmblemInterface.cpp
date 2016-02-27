/***********************************************************************
  ZEmblemInterface.cpp
  
  용  도 : 하드 드라이브에 저장된 엠블램 비트맵을 메모리 캐쉬로 로드하고
           요청시 비트맵 포인터를 넘겨줌.
  작성일 : 18, Nov, 2004
  작성자 : 임동환
************************************************************************/


#include "stdafx.h"							// Include stdafx.h
#include "ZEmblemInterface.h"				// Include ZEmblemInterface.h
#include "MResourceManager.h"				// Include MResourceManager.h
//#include "MEmblemMgr.h"					// Include MEmblemMgr.h
#include "FileInfo.h"
#include "ZGameClient.h"



/***********************************************************************
  ZEmblemInterface : public
  
  desc : 생성자
************************************************************************/
ZEmblemInterface::ZEmblemInterface( void)
{
}


/***********************************************************************
  ~ZEmblemInterface : public
  
  desc : 소멸자
************************************************************************/
ZEmblemInterface::~ZEmblemInterface( void)
{
	Destroy();
}

void ZEmblemInterface::Create(void)
{
	// "no emblem"으로 쓸 비트맵 포인터를 미리 구한다.
	m_pBitmapNoEmblem = MBitmapManager::Get( "no_emblem.png");
}

void ZEmblemInterface::Destroy(void)
{
	ClearClanInfo();
}

/***********************************************************************
  AddClanInfo : public
  
  desc : 클랜 정보를 추가한다. 플레이어가 로그온 할때마다 호출해주면 된다.
  arg  : nClanID = 클랜 ID값
  ret  : true(add) or false(do not add)
************************************************************************/
bool ZEmblemInterface::AddClanInfo( UINT nClanID)
{
	if(nClanID==0) return false;

	// 엠블럼 정보 리스트에서 해당 엠블럼 정보가 있는지 검사한다.
	EmblemInfoMapList::iterator Iterator;
	if ( FindClanInfo( nClanID, &Iterator))
	{
		// 이미 존재하면 해당 엠블럼의 인원수를 한명 늘린다.
		(*Iterator).second.m_nNumOfClan++;

		return ( false);					// return : false
	}
	
	// 리스트에 없으면 노드를 새로 생성한다.
	ZEmblemInfoNode EmblemNode;
	EmblemNode.m_nClanID		= nClanID;
	EmblemNode.m_nNumOfClan		= 1;

	// 리스트에 노드 추가
	m_EmblemInfoMap.insert( EmblemInfoMapList::value_type( nClanID, EmblemNode));

	// bitmap 로드
	ReloadClanInfo(nClanID);

	// 오래되고 사용되지 않는 비트맵을 메모리 캐쉬에서 삭제한다(쓸일이 없을 듯).
	if ( 0)
	{
		for ( EmblemInfoMapList::iterator i = m_EmblemInfoMap.begin();  i != m_EmblemInfoMap.end();  i++)
		{
			if ( (*i).second.m_nNumOfClan <= 1)		// 현재 앰블럼을 사용하는 인원이 1명 이하라면...
				m_EmblemInfoMap.erase( i);			// 리스트에서 삭제한다.
		}
	}

	return ( true);							// return : true
}

bool ZEmblemInterface::ReloadClanInfo(UINT nClanID)
{
	EmblemInfoMapList::iterator Iterator;
	if ( !FindClanInfo( nClanID, &Iterator)) return false;

	ZEmblemInfoNode &EmblemNode = Iterator->second;

	char szFilePath[256];
	if(!ZGetGameClient()->GetEmblemManager()->GetEmblemPathByCLID(nClanID,szFilePath)) 
		return false;

	SAFE_DELETE(EmblemNode.m_pBitmapEmblem);

#ifdef _PUBLISH
	MZFile::SetReadMode( MZIPREADFLAG_ZIP | MZIPREADFLAG_MRS | MZIPREADFLAG_MRS2 | MZIPREADFLAG_FILE );
#endif

	// 클랜ID 값을 이용하여 해당 비트맵을 메모리 캐쉬로 로드한다.
	MBitmapR2 *pBitmap = new MBitmapR2;
	pBitmap->Create("clanEmblem",RGetDevice(),szFilePath,false);
	EmblemNode.m_pBitmapEmblem = pBitmap;

#ifdef _PUBLISH
	MZFile::SetReadMode( MZIPREADFLAG_MRS2 );
#endif

	return true;
}

/***********************************************************************
  DeleteClanInfo : public
  
  desc : 해당 ClanID를 사용하는 인원 수가 0명이면 클랜 정보를 삭제한다.
         플레이어가 로그오프 할때마다 호출해주면 된다.
  arg  : nClanID = 클랜 ID값
  ret  : true(delete) or false(do not delete)
************************************************************************/
bool ZEmblemInterface::DeleteClanInfo( UINT nClanID)
{
	if(nClanID==0) return false;

	// 리스트에서 해당 클랜ID가 있는지 검사한다.
	EmblemInfoMapList::iterator Iterator;
	if ( !FindClanInfo( nClanID, &Iterator))
		return ( false);					// return : false


	// 해당 클랜 엠블럼을 사용하는 인원수를 1명 감소시킨다.
	(*Iterator).second.m_nNumOfClan--;

	// 해당 클랜 엠블럼을 사용하는 인원수가 0명이면 리스트에서 삭제한다.
	if ( (*Iterator).second.m_nNumOfClan == 0)
		m_EmblemInfoMap.erase( Iterator);

	return ( true);							// return : true
}


/***********************************************************************
  ClearClanInfo : public
  
  desc : 클랜 정보를 완전히 초기화 한다.
  arg  : none
  ret  : true(success) or false(fail)
************************************************************************/
bool ZEmblemInterface::ClearClanInfo( void)
{
	m_EmblemInfoMap.clear();

	return ( true);							// return : true
}


/***********************************************************************
  GetClanEmblem : public
  
  desc : 해당 클랜의 비트맵 엠블럼 포인터를 얻는다. 즉시 쓰고 버려야한다
  arg  : nClanID = 클랜 ID값
  ret  : NULL(fail)  or
         Pointer of bitmap(success)
************************************************************************/
MBitmap* ZEmblemInterface::GetClanEmblem( UINT nClanID)
{
	if(nClanID==0) return NULL;

	// 리스트에서 해당 클랜ID가 있는지 검사한다.
	EmblemInfoMapList::iterator Iterator;
	if ( !FindClanInfo( nClanID, &Iterator))
	{
		// 리스트가 없으면 "no emblem" 비트맵 포인터를 리턴한다.
		return m_pBitmapNoEmblem;
	}

	// 리스트에 해당 클랜이 있으면 비트맵 엠블럼 포인터를 구한다.
	return Iterator->second.m_pBitmapEmblem;
}


/***********************************************************************
  GetClanName : public
  
  desc : 해당 클랜의 이름을 얻는다.
  arg  : nClanID = 클랜 ID값
  ret  : NULL(fail)  or
         String of clan name
************************************************************************/
//const char* ZEmblemInterface::GetClanName( UINT nClanID)
//{
//	// 리스트에서 해당 클랜ID가 있는지 검사한다.
//	EmblemInfoMapList::iterator Iterator;
//	if ( !FindClanInfo( nClanID, &Iterator))
//		return ( NULL);						// return : NULL
//
//	// 리스트에 해당 클랜이 있으면 이름값을 구한다.
//	char* szClanName = NULL;
//	strcpy_safe( szClanName, (*Iterator).second.m_szClanName);
//
//	return ( szClanName);					// return : clan name
//}


/***********************************************************************
  FindClanInfo : public
  
  desc : 해당 클랜 정보가 있는지 검사한다. 있으면 리스트 위치를 리턴한다.
  arg  : nClanID = 클랜 ID값
         pIterator = 리턴 받을 iterator 포인터
  ret  : true(found) or false(not found)
************************************************************************/
bool ZEmblemInterface::FindClanInfo( UINT nClanID, EmblemInfoMapList::iterator* pIterator)
{
	EmblemInfoMapList::iterator Iterator;
	Iterator = m_EmblemInfoMap.find( nClanID);

	if ( Iterator == m_EmblemInfoMap.end())
		return ( false);					// return : not found

	*pIterator = Iterator;					// Set list iterator

	return ( true);							// return : found
}
