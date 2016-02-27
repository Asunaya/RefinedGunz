#ifndef MGRIDHASH_H
#define MGRIDHASH_H

#include <crtdbg.h>
#include <list>
#include <math.h>
using namespace std;


/// x-y 그리드맵과 z는 리스트로 가지고 있는 클래스
/// - 선언
///  - MGridMap<UserType>
/// - 오브젝트 추가/이동/삭제
///  - MGridMap<UserType>::HREF h = MGridMap<UserType>::Add(x, y, z, UserTypeObj);
///  - MGridMap<UserType>::Move(x, y, z, UserTypeObj, h);
///  - MGridMap<UserType>::Del(h);
template<class _T>
class MGridMap{
public:
	/// 내부에서 관리하기위한 위치정보와 UserData
	struct MITEM{
		float	x, y, z;	///< Position
		_T		Obj;		///< User Data Object
	};
	/// x-y평면에 놓여지는 하나의 참조 셀
	class MRefCell : public list<MITEM>{};

	/// User Data Object에서 가지고 있을 레퍼런스의 핸들
	struct HREF{
		MRefCell*			pRefCell;
		typename MRefCell::iterator	RefIterator;
	};

protected:
	MRefCell*	m_pGridMap;		///< 그리드맵
	float		m_fSX;			///< 맵의 X축 시작점
	float		m_fSY;			///< 맵의 Y축 시작점
	float		m_fEX;			///< 맵의 X축 끝점
	float		m_fEY;			///< 맵의 Y축 끝점
	//float		GetXSize();		///< 맵의 X축 크기
	//float		GetYSize();		///< 맵의 Y축 크기
	int			m_nXDivision;	///< 맵의 X축 셀 갯수
	int			m_nYDivision;	///< 맵의 Y축 셀 갯수

protected:
	MRefCell* GetCell(float x, float y){
		int nXPos = int((x-m_fSX)/(GetXSize()/(float)m_nXDivision));
		int nYPos = int((y-m_fSY)/(GetYSize()/(float)m_nYDivision));

		// 영영 검사
		_ASSERT(nXPos>=0 && nYPos>=0);
		if(nXPos>=m_nXDivision) return NULL;
		if(nYPos>=m_nYDivision) return NULL;

		return &(m_pGridMap[nXPos+nYPos*m_nXDivision]);
	}
public:
	MGridMap(void){
		m_pGridMap = NULL;
	}
	virtual ~MGridMap(void){
		Destroy();
	}

	/// 맵 생성
	/// @param fSX			맵의 X축 시작점
	/// @param fSY			맵의 Y축 시작점
	/// @param fXSize		맵의 X축 크기
	/// @param fYSize		맵의 Y축 크기
	/// @param nXDivision	맵의 X축 셀 갯수
	/// @param nYDivision	맵의 Y축 셀 갯수
	//void Create(float fSX, float fSY, float fXSize, float fYSize, int nXDivision, int nYDivision){
	void Create(float fSX, float fSY, float fEX, float fEY, int nXDivision, int nYDivision){
		m_pGridMap = new MRefCell[nXDivision*nYDivision];
		m_fSX = fSX;
		m_fSY = fSY;
		m_fEX = fEX;
		m_fEY = fEY;
		//GetXSize() = fXSize;
		//GetYSize() = fYSize;
		m_nXDivision = nXDivision;
		m_nYDivision = nYDivision;
	}
	/// 해제
	void Destroy(void){
		if(m_pGridMap!=NULL){
			delete[] m_pGridMap;
			m_pGridMap = NULL;
		}
	}

	/// 참조 셀에 추가
	/// @return 추가된 오브젝트는 HREF를 보관해서 이동/삭제에 사용해야 한다.
	HREF Add(float x, float y, float z, _T Obj){
		_ASSERT(x>=m_fSX && y>=m_fSY);
		_ASSERT(x<=m_fEX && y<=m_fEY);
		HREF hPos;
		MRefCell* pCell = GetCell(x, y);
		if(pCell==NULL){
			hPos.pRefCell = NULL;
			return hPos;
		}
		MITEM item;
		item.x = x;
		item.y = y;
		item.z = z;
		item.Obj = Obj;
		hPos.RefIterator = pCell->insert(pCell->end(), item);
		hPos.pRefCell = pCell;
		return hPos;
	}
	/// 참조 셀 삭제
	void Del(HREF hRef){
		_ASSERT(hRef.pRefCell!=NULL);
		hRef.pRefCell->erase(hRef.RefIterator);
	}
	/// 해당 영역에 Obj리스트 얻기, Obj리스트는 가까운순으로 소트되어 있지 않다.
	void Get(list<_T>* pObjs, float x, float y, float z, float fRadius){
		float fXCellSize = GetXSize()/(float)m_nXDivision;
		float fYCellSize = GetYSize()/(float)m_nYDivision;
		int nXPos = int((x-m_fSX)/fXCellSize);
		int nYPos = int((y-m_fSY)/fYCellSize);
#define MORE_SEARCH	2
		int nXRadius = int(fRadius/fXCellSize) + MORE_SEARCH;
		int nYRadius = int(fRadius/fYCellSize) + MORE_SEARCH;
		float fRadiusPow = fRadius*fRadius;
		for(int yp=-nYRadius; yp<=nYRadius; yp++){
			for(int xp=-nXRadius; xp<=nXRadius; xp++){
				float fCellX = (nXPos+xp+(xp>=0?1:0))*fXCellSize + m_fSX;
				float fCellY = (nYPos+yp+(yp>=0?1:0))*fYCellSize + m_fSY;
				float f2DLenPow = float(pow(fCellX-x, 2) + pow(fCellY-y, 2));
				if(f2DLenPow>fRadiusPow) continue;	// 셀 자체의 길이 범위가 벗어났을 경우
				int nX = nXPos+xp;
				int nY = nYPos+yp;
				if(nX<0 || nX>=m_nXDivision) continue;
				if(nY<0 || nY>=m_nYDivision) continue;
				MRefCell* pRefCell = &(m_pGridMap[nX+nY*m_nXDivision]);
				for(MRefCell::iterator it=pRefCell->begin(); it!=pRefCell->end(); it++){
					MITEM* pItem = &(*it);
					float f3DLenPow = float(pow(pItem->x-x, 2)+pow(pItem->y-y, 2)+pow(pItem->z-z, 2));
					if(f3DLenPow<=fRadiusPow){
						pObjs->insert(pObjs->end(), pItem->Obj);
					}
				}
			}
		}
	}

	/// Obj 이동에 따른 참조 셀의 이동
	HREF Move(float x, float y, float z, _T Obj, HREF& hRef){
		_ASSERT(hRef.pRefCell!=NULL);
		_ASSERT((&(*hRef.RefIterator))->Obj==Obj);

		MRefCell* pRefCell = GetCell(x, y);
		// 같은 RefCell을 가지면 아무것도 하지 않는다.
		if(pRefCell==hRef.pRefCell) return hRef;
		// 이전 참조 지우기
		hRef.pRefCell->erase(hRef.RefIterator);
		// 새로운 참조 만들기
		return Add(x, y, z, Obj);
	}

	/// 맵의 시작 X
	float GetSX(void) const { return m_fSX; }
	/// 맵의 시작 Y
	float GetSY(void) const { return m_fSY; }
	/// 맵의 끝 X
	float GetEX(void) const { return m_fEX; }
	/// 맵의 끝 Y
	float GetEY(void) const { return m_fEY; }

	/// 맵의 X축 크기
	float GetXSize(void) const { return m_fEX-m_fSX; }
	/// 맵의 Y축 크기
	float GetYSize(void) const { return m_fEY-m_fSY; }
	/// 맵의 X축 셀 갯수
	int GetXDivision(void) const { return m_nXDivision; }
	/// 맵의 Y축 셀 갯수
	int GetYDivision(void) const { return m_nYDivision; }

	/// 맵의 X축 셀 크기
	float GetXDivisionSize(void) const { return GetXSize() / (float)m_nXDivision; }
	/// 맵의 Y축 셀 크기
	float GetYDivisionSize(void) const { return GetYSize() / (float)m_nYDivision; }

	/// x, y 위치의 셀 정보 얻기
	MRefCell* GetCell(int x, int y){
		if(x<0 || x>=m_nXDivision) return NULL;
		if(y<0 || y>=m_nYDivision) return NULL;

		return &(m_pGridMap[x+y*m_nXDivision]);
	}
	/// 인덱스로 셀 정보 얻기
	MRefCell* GetCell(int i){
		if(i<0 || i>=m_nXDivision*m_nYDivision) return NULL;
		return &(m_pGridMap[i]);
	}
	/// 셀 갯수 얻기
	int GetCellCount(void){
		return m_nXDivision*m_nYDivision;
	}
	/// 모든 셀 초기화하기
	void ClearAllCell(void){
		int nCellCount = GetCellCount();
		for(int i=0; i<nCellCount; i++){
			MRefCell* pRefCell = GetCell(i);
			for(MMap::MRefCell::iterator it=pRefCell->begin(); it!=pRefCell->end(); it++){
				MMap::MITEM* pItem = &(*it);
			}
			pRefCell->clear();
		}
	}
};

#endif