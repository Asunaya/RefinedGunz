#ifndef _ZEMBLEMINTERFACE_H
#define _ZEMBLEMINTERFACE_H

/***********************************************************************
  ZEmblemInterface.h
  
  용  도 : 하드 드라이브에 저장된 엠블램 비트맵을 메모리 캐쉬로 로드하고
           요청시 비트맵 포인터를 넘겨줌.
  작성일 : 18, Nov, 2004
  작성자 : 임동환
************************************************************************/


//#pragma once
#include <string>							// Include string
#include <map>								// Include map
#include "MMatchGlobal.h"					// Include MMatchGlobal.h
#include "MBitmap.h"						// Include MBitmap.h
using namespace std;



// Class : ZEmblemMemNode
// 엠블럼 정보 및 클랜 정보를 map list 형식으로 저장하기 위한 노드
struct ZEmblemInfoNode
{
	UINT		m_nClanID;								// Clan ID
	UINT		m_nNumOfClan;							// Number of people in clan
	MBitmap*	m_pBitmapEmblem;						// Clan bitmap emblem

	// Initialize
	ZEmblemInfoNode() {									// Constructor
		m_nClanID			= 0;
		m_nNumOfClan		= 0;
		m_pBitmapEmblem		= NULL;
	}

	virtual ~ZEmblemInfoNode() {
		SAFE_DELETE(m_pBitmapEmblem);
	}
};

typedef  map<int,ZEmblemInfoNode>  EmblemInfoMapList;


// Class : ZEmblemInterface
// 엠블럼 정보를 저장 및 로드하기 위한 클래스
class ZEmblemInterface
{
public:		// public variables


public:		// public functions
	ZEmblemInterface( void);									// Constructor
	~ZEmblemInterface( void);									// Destructor

	void Create(void);
	void Destroy(void);

	bool AddClanInfo( UINT nClanID);							// Add clan infomation
	bool DeleteClanInfo( UINT nClanID);							// Delete clan infomation
	bool ClearClanInfo( void);									// Clear clan infomation
	bool ReloadClanInfo( UINT nClanID);							// 실제로 bitmap을 로드한다

	bool ZEmblemInterface::FindClanInfo( UINT nClanID, EmblemInfoMapList::iterator* pIterator);		// Find clan infomation

	MBitmap* GetClanEmblem( UINT nClanID);						// Get pointer of bitmap emblem

protected:	// protected varialbes
	EmblemInfoMapList	m_EmblemInfoMap;						// Map list of emblem infomation
	MBitmap*			m_pBitmapNoEmblem;						// Bitmap pointer of "no emblem"

protected:	// protected functions
};


#endif