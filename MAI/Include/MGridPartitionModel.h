/*

  바둑판 모형의 맵을 파티셔닝한 모델
  ----------------------------------

  2000/03/02, lee jang ho
  All copyright (c) 1998, MAIET entertainment software, inc

*/
#ifndef MGridPartitionModel_H
#define MGridPartitionModel_H

#include "MAStar.h"


struct MGPPOINT{
	int x, y;
};
struct MGPRECT{
	int x, y;
	int w, h;
};
struct MGPLINE{
	MGPPOINT p[2];
};


class MGridPartition;


// MGridPartition포인터 이외의 부가 정보를 포함
class MGridPartitionLink{
public:
	MGridPartition*		m_pGP;
	MGPLINE				m_Portal;		// m_pGP로 가는 포탈
	int					m_nDivision;	// m_pGP로 가는 포탈의 Division ( 0 ~ 4 )
};

// 공간을 나누는 Partition
class MGridPartition : public MNodeModel{
	CMLinkedList<MGridPartitionLink>	m_Successors;
public:
	MGPRECT	m_Rect;

protected:
	void GetCenter(float* x, float* y);

public:
	MGridPartition(int x, int y, int w, int h);
	virtual ~MGridPartition(void);

	bool HasSuccessor(MGridPartition* pGP);
	void AddSuccessor(MGridPartition* pGP);
	// 이웃 Node 얻기
	virtual int GetSuccessorCount(void);
	virtual MNodeModel* GetSuccessor(int i);
	virtual float GetSuccessorCost(MNodeModel* pSuccessor);
	virtual float GetHeuristicCost(MNodeModel* pNode);
	MGridPartitionLink* GetLink(MGridPartition* pGP);

	bool GetOutLines(CMLinkedList<MGPLINE>* pLines, CMPtrList<MGridPartition>* pGPs);
};


struct MGPMCELL{
	bool				bEnable;	// 갈 수 있는 Cell인가?
	MGridPartition*		pPart;		// 이 Cell이 속해 있는 Partition
};


class MGridPartitionModel{
	MGPMCELL*	m_pCells;						// 셀 단위(바둑판 모양) 맵
	int			m_nWidth, m_nHeight;
public:
	CMLinkedList<MGridPartition>	m_Parts;	// 파티션 ( 직사각형 모양을 가지고 있다 )

protected:
	// 아직 파티션이 안된 구역의 최대 영역 구하기
	bool IsEmptyRangeWidth(int x, int y, int w);
	bool IsEmptyRangeHeight(int x, int y, int h);
	void GetEmptyRangeWidthFirst(int* w, int* h, int x, int y);		// Width를 먼저 고려
	void GetEmptyRangeHeightFirst(int* w, int* h, int x, int y);	// Height를 먼저 고려
	int GetEmptySquareSideLength(int x, int y);						// 정방형 빈 영역을 찾는다.

	void FillPartition(int x, int y, int w, int h, MGridPartition* pGP);
public:
	MGridPartitionModel(void);
	virtual ~MGridPartitionModel(void);

	bool Create(int x, int y);
	void Destroy(void);

	MGPMCELL* Get(int x, int y);

	void Enable(int x, int y, bool bEnable=true);

	void Partition(void);
};

// 패스파인더 경로(MGridPartition)로 아웃 라인 얻어내기
void GenerateOutLine(CMLinkedList<MGPLINE>* pLines, MAStar* pPF, int nLimitPath=-1);


#endif