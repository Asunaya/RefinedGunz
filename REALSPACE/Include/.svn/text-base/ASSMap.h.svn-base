#ifndef __ASSMAP_H
#define __ASSMAP_H

#include <stdio.h>
#include "RSTypes.h"
#include "rsd3d.h"

#define ASSMAPBASE						6969
#define AMCHUNKHEADERID_SIZE			ASSMAPBASE
#define AMCHUNKHEADERID_END				ASSMAPBASE+1
#define AMCHUNKHEADERID_GEOMETRY		ASSMAPBASE+2
#define AMCHUNKHEADERID_LODTEXTURE		ASSMAPBASE+3
#define AMCHUNKHEADERID_LODTEXTURE2		ASSMAPBASE+4
#define AMCHUNKHEADERID_MINIMAPTEXTURE	ASSMAPBASE+5
#define AMCHUNKHEADERID_COLOR			ASSMAPBASE+6

#define LODCELL_TEXSIZE	2
#define QUEUECOUNT	20000
#define BUFFERCOUNT	2000

enum ASSMAPSIZE
{
	ASSMAPSIZE_32	= 33,
	ASSMAPSIZE_64	= 65,
	ASSMAPSIZE_128	= 129,
};

// 파일 저장/읽기용 스트럭쳐
#pragma pack(1)
struct ASSCELLFLAGS{
	unsigned short int nTextureIndex	: 16;
	unsigned char nTextureDir	: 2;	// 지금은 4방향으로 다 해결. 확장이 필요할지도...
	unsigned short int nTileSet		: 14;	// 줄여도 됨.
};
#pragma pack()

enum INOUTSTATE {
	IS_IN,
	IS_OUT,
	IS_DONTKNOW
};

#define RSMFVF	( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX2 )

typedef struct MVERTEX {
	float x, y, z;		// world position
	DWORD Diffuse;   // diffuse color    
	DWORD Specular;  // specular color    
	float tu1, tv1;  // texture coordinates
	float tu2, tv2;  // texture coordinates
} MVERTEX, *LPMVERTEX; 

struct ASSCell
{
	rvector	normal;
	rplane planes[2];
	WORD *pind;
	int		m_nTileSet;
	int		m_nTextureIndex,m_nTextureDir;
	float	m_fFow;
	rvector	m_color;
	DWORD	m_dwColor;
	int		m_nTransformedFrame,m_nLitFrame;

	ASSCell(){	m_fFow=1.0f;m_nTileSet=0;m_nLitFrame=0;m_nTransformedFrame=0; }
	virtual ~ASSCell(){}
};

typedef CMLinkedList<ASSCell> RSCellList;

struct ASSVolumeTreeNode;

struct ASSLODInfo
{
	ASSCell *pCells[4];
	int CrackPoint[4];
	ASSVolumeTreeNode *m_pAdjacencies[4];
};

struct ASSVolumeTreeNode
{
	rboundingbox	m_bbTree;
	rvector			m_Center;
	float			m_fRadius,m_fVariance;

	ASSCell			*m_pCell;
	ASSLODInfo		*m_pLODInfo;
	INOUTSTATE		states[6];
	bool			m_bAllIn,m_bExpanded;
	int				m_nChildren[4];
	int				nRefTexture;
	int				nLevel;
	WORD			ind[6];

	ASSVolumeTreeNode();
	virtual ~ASSVolumeTreeNode();
};

class ASSMap
{
public:
	ASSMap();
	virtual ~ASSMap();

	virtual bool Open(FILE *file,ASSSchemeManager *pSchemeManager);
	virtual bool Open(const char* filename,ASSSchemeManager *pSchemeManager);
	virtual void Draw();

	virtual void Invalidate();
	virtual void Restore();

	void DrawLine();
	void DrawLightMap(RSPointLight *light);
	void ProcessVertexLight(RSPointLight *light);

	int GetMapSizeX() { return nx-1; }
	int GetMapSizeY() { return ny-1; }

	void SetLODState(bool bEnable,float fLODBias=1.f);
	bool GetLODState() { return m_bLODEnable; }
	int	GetMinimapTextureHandle() { return m_hMinimapTexture; }

//	rface *GetFace(float x,float y);
/*
		iface ::
		+-------+-------+
		| 0	  /	| \	  1 |
		|   /	|   \	|
		| /   1 | 0   \ |
		+-------+-------1
*/
//	rface *GetFace(int x,int y,int iface);

/*
		GetCase() ::
		+-------+-------+-------+-------+
		| 0	  /	| \	  3 | 0	  /	| \	    |
		|   /	|   \	|   /	|   \	|
		| /   1 | 2   \ | /   1 |     \ |
		+-------0-------1-------+-------+
		| \	  3	| 0   /	| \	  3	| 	  /	|
		|   \	|   /	|   \	|   /	|
		|  2  \ | /   1 | 2   \ | /     |
		+-------+-------+-------+-------+
*/
	// vertex의 경우
	inline int GetCase(int x,int y) { return ((x+y)%2) ? 1 : 0 ; }
	inline int GetCase(int index) { return GetCase(index % nx , index / nx ); }

	// face의 경우
	inline int GetCase(float x,float y) {
		return (((int)x+(int)y)%2) ?
					(x-(int)x<y-(int)y) ? 2 : 3			:
					(x-(int)x+y-(int)y<1) ? 0 : 1;
	}

	inline ASSCell *GetCell(int x,int y) { return ((x<0)||(y<0)||(x>=nx-1)||(y>=ny-1)) ? NULL : &m_Cells[y*nx+x]; }
	inline ASSCell *GetCell(float x,float y) { return GetCell(int(x),int(y)); }
	inline ASSCell *GetCell(int index) { return ((index<0)||(index>=nx*ny)) ? NULL : &m_Cells[index]; }

	bool Pick(const rvector *origin,const rvector *target,rvector *ret);	// Pick With Line
	bool Pick(rvector &origin,rvector &target,rvector *ret);
	bool Pick(int sx, int sy, rvector *ret);								// Pick with Screen Coordinates

	rvector GetInterpolatedNormal(float x, float y);
	rvector GetNormal(float x,float y);
	float GetFOW(float,float);
	void SetFOW(int,int,float);
	void SetConstantFOW(float);
	float GetHeight(float x,float y);
	float GetHeight(int x,int y);
	inline float GetHeight(int index) { return m_Heights[index]; }
	inline rvector GetVector(int x,int y) { return rvector((float)x,(float)y,GetHeight(x,y)); }
	inline rvector GetVector(int index) { return GetVector(index % nx , index / nx); }
	inline rvector GetVector(float x,float y) { return GetVector(int(x),int(y)); }
	void SetHeight(int,int,float);				// 높이를 바꾸면 그 부근을 UpdateGeometry해줘야한다.
	void UpdateColors(ASSVolumeTreeNode *node,int x1,int y1,int x2,int y2);
	void UpdateGeometry(int x1,int y1,int x2,int y2);
	void SetTileSet(int x,int y,int nTileSet);	// tileset을 바꾸면 UpdateMapping.
	void UpdateMapping_Recurse(ASSVolumeTreeNode *node,int x1,int y1,int x2,int y2);
	void UpdateMapping(int x1,int y1,int x2,int y2);
	void GetCellListWRTPlanes(RSCellList *fl,rplane *planes,int nplanes);
	void DrawBlackMark(float xPos,float yPos,float Radius,DWORD color,DWORD hTexture);

// fog 관련
	float m_FogStartZ,m_FogEndZ,m_1oDistance;
	DWORD GetFogValue(float z);
	void SetFogRange(float sz,float ez) 
		{ m_FogStartZ=sz;m_FogEndZ=ez;m_1oDistance=1.f/(m_FogEndZ-m_FogStartZ); }

protected:
// Scheme manager
	ASSSchemeManager *m_pSchemeManager;
	int	m_hPattern;

	virtual inline void CalcColor(int index);
	virtual inline void Transform(int index);
	ASSVolumeTreeNode *ConstructVolumeTree(ASSVolumeTreeNode *pParent,int x1,int y1,int x2,int y2,int depth=0);
	void TraverseTreeAndChoose(ASSVolumeTreeNode *node);
	void DrawLightMap(rface *f,RSPointLight *light);
	bool UpdateInOutStates(ASSVolumeTreeNode *parent,ASSVolumeTreeNode *child);

	// members
	int					nx,ny;
	ASSCell				*m_Cells;
	DWORD*				m_pCellColor;				
	int					m_hMinimapTexture,m_nPatches_x,m_nPatches_y,*m_hLODTextures;
	int					*m_LodTextures,m_nlodcell_texsize;
	bool				m_bLODEnable;
	float				*m_Heights;
	float				*m_LODTable;

	// Private Helpers
	void UpdateNormals(int x1,int y1,int x2,int y2);
	void UpdateBoundingVolumes(ASSVolumeTreeNode *node,int x1,int y1,int x2,int y2);
	bool Pick(ASSVolumeTreeNode *node,rvector *ret);

	// 읽기위한 functions
	void ReadChunk_Size(FILE *file);
	void ReadChunk_Color(FILE *file);
	void ReadChunk_Geometry(FILE *file);
	void ReadChunk_LODTexture(FILE *file);
	void ReadChunk_LODTexture2(FILE *file);
	void ReadChunk_MinimapTexture(FILE *file);

	int m_nTreeNode,m_nTreeDepth;
	int m_nSchemeTextures,nQueueCount;
	int *nRefTextures,*nQueueCounts;
	ASSVolumeTreeNode ***m_pQueues;

	int nQ;
	ASSVolumeTreeNode *queue[QUEUECOUNT];

//#ifndef EL_TEST

	MVERTEX	*pVertices;
	LPDIRECT3DVERTEXBUFFER8	pVertexBuffer;
	LPDIRECT3DINDEXBUFFER8	pIndexBuffer;
	LPDIRECT3DINDEXBUFFER8	*ppIndexBuffers;

//#else

	MVERTEX*	m_point_list;
	WORD*		m_index_list;

//#endif
	
	void CreateSchemeRefTextures();
	void CreateVertexBuffer();

	ASSVolumeTreeNode *m_pTree;
	void ConstructVolumeTree();
	void ConstructVolumeTree(int index,int x1,int y1,int x2,int y2);
	void Merge(int index);
	void Split(int index);
	void MarkInvisible(int index);
	void Draw(int index);
	void Enque(ASSVolumeTreeNode *pNode);
	void Flush(int nRefTexture);
	void Draw(ASSVolumeTreeNode *pNode);
	ASSVolumeTreeNode *GetAdjacency(int index,int nAdj);

public:


//#ifndef EL_TEST

	LPDIRECT3DVERTEXBUFFER8	GetVertexBuffer() { return pVertexBuffer; }
	LPDIRECT3DINDEXBUFFER8 GetIndexBuffer() { return pIndexBuffer; }

//#else
	
	MVERTEX*	GetVertexList() {return m_point_list;}
	WORD*		GetIndexList()  {return m_index_list;}

//#endif
	
};

#endif