#ifndef MCOLLISIONMAPGENERATOR_H
#define MCOLLISIONMAPGENERATOR_H

#pragma warning(disable:4786)

#include <list>

using namespace std;

struct MCOLPOINT{
	int x, y;
};

struct MCOLCELL{
	int	x, y;

	MCOLCELL(void){
		x = y = 0;
	}
	MCOLCELL(int x, int y){
		MCOLCELL::x = x;
		MCOLCELL::y = y;
	}
};


typedef list<MCOLCELL*>		MColCellList;

class MColPolygon : public list<MCOLPOINT>{
public:
	int				sx, sy, ex, ey;		// Bounding Box
public:
	MColPolygon(void);
	virtual ~MColPolygon(void);
	void CalcBoundingBox(void);
};

class MColPolygonList : public list<MColPolygon*>{
public:
	MColPolygonList(void);
	virtual ~MColPolygonList(void);
	void CalcBoundingBox(void);
};

class MGridCollisionBlock{
public:
	list<MCOLCELL*>	m_Cells;

	virtual ~MGridCollisionBlock(void){
		while(m_Cells.empty()==false){
			MCOLCELL* pCell = *(m_Cells.begin());
			delete pCell;
			m_Cells.erase(m_Cells.begin());
		}
	}

	void Add(int x, int y){
		m_Cells.insert(m_Cells.end(), new MCOLCELL(x, y));
	}

	bool IsCell(int x, int y);

	void GenerateEdgePolygon(MColPolygon* p, int x, int y, int nDirection);
	void GenerateEdgePolygon(MColPolygonList* pPL);

	bool IsExist(MColPolygonList* pPL, MCOLPOINT p[2]);
};

class MColBlockList : public list<MGridCollisionBlock*>{
public:
	virtual ~MColBlockList(void){
		while(empty()==false){
			delete (*begin());
			erase(begin());
		}
	}
};


class MGridCollisionMap{
protected:
	void AddCell(MGridCollisionBlock* pBlock, int x, int y, list<MCOLCELL>* pCL, bool* pTestMap);
	void ExtendBlock(MGridCollisionBlock* pBlock, int x, int y, bool* pTestMap);
public:
	virtual int GetWidth(void) = 0;
	virtual int GetHeight(void) = 0;

	virtual bool IsCollision(int x, int y) = 0;

	void GenerateCollisionBlock(MColBlockList* pCollisionBlocks);
};

#endif