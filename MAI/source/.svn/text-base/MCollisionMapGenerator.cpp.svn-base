#include "MCollisionMapGenerator.h"
#include <crtdbg.h>

MColPolygon::MColPolygon(void)
{
	sx = sy = 1000000;
	ex = ey = -1000000;
}

MColPolygon::~MColPolygon(void)
{
}

void MColPolygon::CalcBoundingBox(void)
{
	for(MColPolygon::iterator i=begin(); i!=end(); i++){
		MCOLPOINT p = *i;
		if(p.x<sx) sx = p.x;
		if(p.y<sy) sy = p.y;
		if(p.x>ex) ex = p.x;
		if(p.y>ey) ey = p.y;
	}
}


MColPolygonList::MColPolygonList(void)
{
}

MColPolygonList::~MColPolygonList(void)
{
	/*
	while(empty()==false){
		delete (*begin());
		erase(begin());
	}
	*/
}

void MColPolygonList::CalcBoundingBox(void)
{
	for(iterator i=begin(); i!=end(); i++){
		MColPolygon* pPolygon = *i;
		pPolygon->CalcBoundingBox();
	}
}

bool MGridCollisionBlock::IsCell(int x, int y)
{
	for(list<MCOLCELL*>::iterator i=m_Cells.begin(); i!=m_Cells.end(); i++){
		MCOLCELL* c = (*i);
		if(c->x==x && c->y==y) return true;
	}

	return false;
}

/*
void MGridCollisionBlock::GenerateEdgePolygon(MColPolygon* pl)
{
	MCOLCELL* c = *m_Cells.begin();

	MCOLPOINT cp;			// Current Point
	cp.x = c->x;
	cp.y = c->y;
	MCOLPOINT sp = cp;	// Start Point
	MCOLPOINT pp = cp;	// Previous Point

	pl->insert(pl->end(), cp);

	int nPrevDir = 0;

	while(1){
		/*

		Line Direction

		    0
		    |
		3---+---1
		    |
		    2

		Block Position

		+---+---+
		| 0 | 1 |
		+---+---+
		| 3 | 2 |
		+---+---+

		*/
/*		int x = cp.x;
		int y = cp.y;

		MCOLPOINT np;		// Next Point

		// 4 Block
		bool cm[4];
		cm[0] = IsCell(x-1, y-1);
		cm[1] = IsCell(x, y-1);
		cm[2] = IsCell(x, y);
		cm[3] = IsCell(x-1, y);

		int nNextDir = -1;
		for(int i=0; i<4; i++){
			int nTestDir = (nPrevDir+i+3)%4;	// 테스트 방향의 우선순위 결정
			switch(nTestDir){
			case 0:
				if(cm[0]==false && cm[1]==true){
					nNextDir = 0;
				}
				break;
			case 1:
				if(cm[1]==false && cm[2]==true){
					nNextDir = 1;
				}
				break;
			case 2:
				if(cm[2]==false && cm[3]==true){
					nNextDir = 2;
				}
				break;
			case 3:
				if(cm[3]==false && cm[0]==true){
					nNextDir = 3;
				}
				break;
			}
			if(nNextDir!=-1) break;
		}

		_ASSERT(nNextDir!=-1);

		if(nNextDir==0){
			np.x = x;
			np.y = y-1;
		}
		else if(nNextDir==1){
			np.x = x+1;
			np.y = y;
		}
		else if(nNextDir==2){
			np.x = x;
			np.y = y+1;
		}
		else if(nNextDir==3){
			np.x = x-1;
			np.y = y;
		}

		// 처음과 같은 포인트일때...
		if(cp.x==sp.x && cp.y==sp.y){
			list<MCOLPOINT>::iterator pi = pl->begin();
			pi++;
			
			// 다음 포인트도 같다면 여기서 폴리곤 종료.
			if(np.x==(*pi).x && np.y==(*pi).y){
				break;
			}
		}

		pp = cp;
		cp = np;

		pl->insert(pl->end(), cp);

		nPrevDir = nNextDir;
	}
	

	if(pl->begin()==pl->end()) return;

	// 같은 방향의 포인트 제거
	MColPolygon::iterator i=pl->begin();
	MColPolygon::iterator pi=i;
	i++;

	class local{
	public:
		static int GetDir(MCOLPOINT& p1, MCOLPOINT& p2){
			int x = p2.x - p1.x;
			int y = p2.y - p1.y;
			if(y<0) return 0;
			else if(x>0) return 1;
			else if(y>0) return 2;
			else if(x<0) return 3;
			_ASSERT(false);
			return -1;
		}
	};

	nPrevDir = local::GetDir(*pi, *i);
	pi = i;
	i++;

	while(i!=pl->end()){
		int nThisDir = local::GetDir(*pi, *i);

		if(nThisDir==nPrevDir){
			pl->erase(pi);
		}

		nPrevDir = nThisDir;
		pi = i;
		i++;
	}
}
*/
void MGridCollisionBlock::GenerateEdgePolygon(MColPolygon* pl, int x, int y, int nDirection)
{
	MCOLPOINT cp;			// Current Point
	cp.x = x;
	cp.y = y;
	MCOLPOINT sp = cp;	// Start Point
	MCOLPOINT pp = cp;	// Previous Point

	pl->insert(pl->end(), cp);

	int nPrevDir = (nDirection+1)%4;

	while(1){
		/*

		Line Direction

		    0
		    |
		3---+---1
		    |
		    2

		Block Position

		+---+---+
		| 0 | 1 |
		+---+---+
		| 3 | 2 |
		+---+---+

		*/
		int x = cp.x;
		int y = cp.y;

		MCOLPOINT np;		// Next Point

		// 4 Block
		bool cm[4];
		cm[0] = IsCell(x-1, y-1);
		cm[1] = IsCell(x, y-1);
		cm[2] = IsCell(x, y);
		cm[3] = IsCell(x-1, y);

		int nNextDir = -1;
		for(int i=0; i<4; i++){
			int nTestDir = (nPrevDir+i+3)%4;	// 테스트 방향의 우선순위 결정
			switch(nTestDir){
			case 0:
				if(cm[0]==false && cm[1]==true){
					nNextDir = 0;
				}
				break;
			case 1:
				if(cm[1]==false && cm[2]==true){
					nNextDir = 1;
				}
				break;
			case 2:
				if(cm[2]==false && cm[3]==true){
					nNextDir = 2;
				}
				break;
			case 3:
				if(cm[3]==false && cm[0]==true){
					nNextDir = 3;
				}
				break;
			}
			if(nNextDir!=-1) break;
		}

		_ASSERT(nNextDir!=-1);

		if(nNextDir==0){
			np.x = x;
			np.y = y-1;
		}
		else if(nNextDir==1){
			np.x = x+1;
			np.y = y;
		}
		else if(nNextDir==2){
			np.x = x;
			np.y = y+1;
		}
		else if(nNextDir==3){
			np.x = x-1;
			np.y = y;
		}

		// 처음과 같은 포인트일때...
		if(cp.x==sp.x && cp.y==sp.y){
			list<MCOLPOINT>::iterator pi = pl->begin();
			pi++;
			
			// 다음 포인트도 같다면 여기서 폴리곤 종료.
			if(np.x==(*pi).x && np.y==(*pi).y){
				break;
			}
		}

		pp = cp;
		cp = np;

		pl->insert(pl->end(), cp);

		nPrevDir = nNextDir;
	}
	

	if(pl->begin()==pl->end()) return;

	// 같은 방향의 포인트 제거
	MColPolygon::iterator i=pl->begin();
	MColPolygon::iterator pi=i;
	i++;

	class local{
	public:
		static int GetDir(MCOLPOINT& p1, MCOLPOINT& p2){
			int x = p2.x - p1.x;
			int y = p2.y - p1.y;
			if(y<0) return 0;
			else if(x>0) return 1;
			else if(y>0) return 2;
			else if(x<0) return 3;
			_ASSERT(false);
			return -1;
		}
	};

	nPrevDir = local::GetDir(*pi, *i);
	pi = i;
	i++;

	while(i!=pl->end()){
		int nThisDir = local::GetDir(*pi, *i);

		if(nThisDir==nPrevDir){
			pl->erase(pi);
		}

		nPrevDir = nThisDir;
		pi = i;
		i++;
	}
}

/*
void MGridCollisionMap::GetCellPolygon(MColPolygonList* p)
{
	for(list<MCOLCELL*>::iterator i=m_Cells.begin(); i!=m_Cells.end(); i++){
		MCOLCELL* c = (*i);
		MColPolygon* pNew = new MColPolygon;
		pNew->insert(pNew->end(), MCOLPOINT(c->x, c->y));
		pNew->insert(pNew->end(), MCOLPOINT(c->x+1, c->y));
		pNew->insert(pNew->end(), MCOLPOINT(c->x+1, c->y+1));
		pNew->insert(pNew->end(), MCOLPOINT(c->x, c->y+1));
		p->insert(p->begin(), pNew);
	}
}
*/
void MGridCollisionBlock::GenerateEdgePolygon(MColPolygonList* pPL)
{
	MCOLCELL* c = *m_Cells.begin();
	MColPolygon* pPointList = new MColPolygon;
	GenerateEdgePolygon(pPointList, c->x, c->y, 0);
	pPL->insert(pPL->end(), pPointList);

	for(list<MCOLCELL*>::iterator i=m_Cells.begin(); i!=m_Cells.end(); i++){
		MCOLCELL* c = (*i);
		int x = c->x;
		int y = c->y;
		/*
		    +---+
		    | 0 |
		+---+---+---+
		| 3 |   | 1 |
		+---+---+---+
		    | 2 |
		    +---+
		*/
		bool b;
		b = IsCell(x, y-1);
		if(b==false){
			MCOLPOINT p[2] = {{x, y}, {x+1, y}};
			if(IsExist(pPL, p)==false){
				MColPolygon* pPointList = new MColPolygon;
				GenerateEdgePolygon(pPointList, x, y, 1);
				pPL->insert(pPL->end(), pPointList);
			}
		}
		b = IsCell(x+1, y);
		if(b==false){
			MCOLPOINT p[2] = {{x+1, y}, {x+1, y+1}};
			if(IsExist(pPL, p)==false){
				MColPolygon* pPointList = new MColPolygon;
				GenerateEdgePolygon(pPointList, x+1, y, 2);
				pPL->insert(pPL->end(), pPointList);
			}
		}
		b = IsCell(x, y+1);
		if(b==false){
			MCOLPOINT p[2] = {{x+1, y+1}, {x, y+1}};
			if(IsExist(pPL, p)==false){
				MColPolygon* pPointList = new MColPolygon;
				GenerateEdgePolygon(pPointList, x+1, y+1, 3);
				pPL->insert(pPL->end(), pPointList);
			}
		}
		b = IsCell(x-1, y);
		if(b==false){
			MCOLPOINT p[2] = {{x, y+1}, {x, y}};
			if(IsExist(pPL, p)==false){
				MColPolygon* pPointList = new MColPolygon;
				GenerateEdgePolygon(pPointList, x, y+1, 0);
				pPL->insert(pPL->end(), pPointList);
			}
		}
	}
}

#define min(_a, _b)	((_a<_b)?(_a):(_b))
#define max(_a, _b)	((_a>_b)?(_a):(_b))

bool MGridCollisionBlock::IsExist(MColPolygonList* pPL, MCOLPOINT p[2])
{
	for(MColPolygonList::iterator i=pPL->begin(); i!=pPL->end(); i++){
		MColPolygon* pP = *i;
		for(MColPolygon::iterator j=pP->begin(); j!=pP->end(); j++){
			MCOLPOINT p1 = *j;
			MColPolygon::iterator k = j; k++;
			//if(k==pP->end()) k = pP->begin();
			if(k==pP->end()) break;
			MCOLPOINT p2 = *k;
			if(p1.y==p2.y){			// 수평 라인
				if(p1.y==p[0].y && p1.y==p[1].y){
					int m = min(p1.x, p2.x);
					int M = max(p1.x, p2.x);
					int tm = min(p[0].x, p[1].x);
					int tM = max(p[0].x, p[1].x);
					if(tm>=m && tM<=M) return true;
				}
			}
			else if(p1.x==p2.x){	// 수직 라인
				if(p1.x==p[0].x && p1.x==p[1].x){
					int m = min(p1.y, p2.y);
					int M = max(p1.y, p2.y);
					int tm = min(p[0].y, p[1].y);
					int tM = max(p[0].y, p[1].y);
					if(tm>=m && tM<=M) return true;
				}
			}
			else{
				// 수직 혹은 수평 라인이여야 한다.
				_ASSERT(false);
			}
		}
	}
	return false;
}

void MGridCollisionMap::AddCell(MGridCollisionBlock* pBlock, int x, int y, list<MCOLCELL>* pCL, bool* pTestMap)
{
	if(x<0 || x>=GetWidth()) return;
	if(y<0 || y>=GetHeight()) return;

	if(pTestMap[x+y*GetWidth()]==true) return;

	pTestMap[x+y*GetWidth()] = true;

	if(IsCollision(x, y)==false) return;

	pBlock->Add(x, y);
	pCL->insert(pCL->end(), MCOLCELL(x, y));
}

void MGridCollisionMap::ExtendBlock(MGridCollisionBlock* pBlock, int x, int y, bool* pTestMap)
{
	list<MCOLCELL> cl;

	cl.insert(cl.end(), MCOLCELL(x, y));

	pBlock->Add(x, y);

	while(cl.empty()==false){
		MCOLCELL c = *(cl.begin());

		// 8 Dir
		AddCell(pBlock, c.x-1, c.y-1, &cl, pTestMap);
		AddCell(pBlock, c.x, c.y-1, &cl, pTestMap);
		AddCell(pBlock, c.x+1, c.y-1, &cl, pTestMap);
		AddCell(pBlock, c.x+1, c.y, &cl, pTestMap);
		AddCell(pBlock, c.x+1, c.y+1, &cl, pTestMap);
		AddCell(pBlock, c.x, c.y+1, &cl, pTestMap);
		AddCell(pBlock, c.x-1, c.y+1, &cl, pTestMap);
		AddCell(pBlock, c.x-1, c.y, &cl, pTestMap);

		cl.erase(cl.begin());
	}
}

void MGridCollisionMap::GenerateCollisionBlock(MColBlockList* pCollisionBlocks)
{
	bool* pTestMap = new bool[GetWidth()*GetHeight()];
	memset(pTestMap, 0, GetWidth()*GetHeight()*sizeof(bool));
	for(int y=0; y<GetHeight(); y++){
		for(int x=0; x<GetWidth(); x++){
			if(pTestMap[x+y*GetWidth()]==true) continue;

			pTestMap[x+y*GetWidth()] = true;

			if(IsCollision(x, y)==true){
				MGridCollisionBlock* pNewBlock = new MGridCollisionBlock;
				ExtendBlock(pNewBlock, x, y, pTestMap);
				pCollisionBlocks->insert(pCollisionBlocks->end(), pNewBlock);
			}
		}
	}
	delete[] pTestMap;
}
