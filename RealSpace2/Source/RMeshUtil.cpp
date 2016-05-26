#include "stdafx.h"
#include "RMeshUtil.h"
#include "RealSpace2.h"
#include "MDebug.h"

/////////////////////////////////////////////////////////////////////////
// RIBuffer

using namespace RealSpace2;

/////////////////////////////////////////////////////////////////////////
// RVBuffer

RIndexBuffer::RIndexBuffer()
{
	m_size = 0;
	m_ib = NULL;
	m_i = NULL;

	m_pIndex = NULL;

	m_dwUsage	 = D3DUSAGE_WRITEONLY;
	m_dwPool	 = D3DPOOL_MANAGED;
	m_dwLockFlag = 0;

	// 용량이 얼마 안하니... 아주 저사양이 아니라면 둘다생성...

	m_bUseSWVertex = false;
	m_bUseHWVertex = false;
}

RIndexBuffer::~RIndexBuffer() 
{
	REL(m_ib);
	DEL2(m_pIndex);
}

void RIndexBuffer::Lock() 
{
	m_ib->Lock( 0, sizeof(WORD)*m_size, (VOID**)&m_i, m_dwLockFlag );
}

void RIndexBuffer::Unlock() 
{
	m_ib->Unlock();
}

int RIndexBuffer::GetFaceCnt() 
{
	if(m_size)	
		return m_size / 3;
	return 0;
}

void RIndexBuffer::Update(int size,WORD* pData) 
{
	if(size!=m_size) 
		return;

	if(m_bUseHWVertex && m_ib) {

		Lock();

		memcpy(m_i,pData,sizeof(WORD)*m_size);

		Unlock();
	}

	if(m_bUseSWVertex && m_pIndex)
		memcpy(m_pIndex,pData,sizeof(WORD)*m_size);
}

bool RIndexBuffer::Create(int size,WORD* pData,DWORD flag,DWORD Usage,D3DPOOL Pool)
{
	m_size = size;

	if(flag & USE_VERTEX_HW) m_bUseHWVertex = true;
	if(flag & USE_VERTEX_SW) m_bUseSWVertex = true;

	if(m_bUseHWVertex) {
		if( FAILED( RGetDevice()->CreateIndexBuffer( sizeof(WORD)*size, Usage , D3DFMT_INDEX16 , Pool, &m_ib ,0) ) ) {
//		return false; 
//		soft 로 돌수도 있다..
			mlog("RIndexBuffer::Create Error : use soft index buffer\n");
		}
	}

	if(m_bUseSWVertex)
		m_pIndex = new WORD[size];

	if(pData)
		Update(size,pData);

	return true;
}

void RIndexBuffer::SetIndices()
{
	RGetDevice()->SetIndices(m_ib);
}

//////////////////////////////////////////////////////////////////

RVertexBuffer::RVertexBuffer()
{
	Init();
}

RVertexBuffer::~RVertexBuffer()
{
	Clear();
}

void RVertexBuffer::Init() {

	m_is_init = false;
	m_vb = NULL;
	m_nVertexSize = 0;
	m_nVertexCnt = 0;
	m_nBufferSize = 0;
	m_nRealBufferSize = 0;
	m_v = NULL;
	m_dwFVF = 0;
	m_dwUsage = D3DUSAGE_WRITEONLY;
	m_dwPool = D3DPOOL_MANAGED;
	m_dwLockFlag = 0;

	m_PrimitiveType = D3DPT_TRIANGLELIST;

	m_nRenderCnt = 0;
	m_pVert = NULL;
	m_bUseSWVertex = false;//기본은 둘다사용,. 옵션에서 바꿀수도 있으므로..
	m_bUseHWVertex = false;
}

void RVertexBuffer::Clear() {

	REL(m_vb);
	DEL(m_pVert);

	Init();
}

// size는 vertex * size 값
bool RVertexBuffer::Create(char* pVertex,DWORD fvf,int VertexSize,int VertexCnt,DWORD flag,DWORD Usage,D3DPOOL Pool) 
{
	Clear();

	// 기본적인 정보는 채워주고

	m_nVertexSize = VertexSize;
	m_nVertexCnt = VertexCnt;
	m_nBufferSize = VertexSize*VertexCnt;
	m_nRealBufferSize = m_nBufferSize;

	m_nRenderCnt = m_nVertexCnt/3;

	m_dwFVF = fvf;
	m_dwUsage = Usage;
	m_dwPool = Pool;

	if(flag & USE_VERTEX_SW) m_bUseSWVertex = true;
	if(flag & USE_VERTEX_HW) m_bUseHWVertex = true;

	// 버퍼는 필요에 의해서 채우고

	if(m_bUseHWVertex) {
		if( FAILED( RGetDevice()->CreateVertexBuffer( m_nBufferSize , Usage , fvf , Pool ,&m_vb ,0) ) ) {
//			return false; // tnt 등의 경우 soft 로 돌수도 있다..
		}
	}

	if(m_bUseSWVertex) {
		m_pVert = new char[m_nBufferSize];
	}

	m_is_init = true;

	if(pVertex)
		Update(pVertex,fvf,VertexSize,VertexCnt);

	return true;
}

bool RVertexBuffer::UpdateDataSW(char* pVertex)
{
	if(m_bUseSWVertex && m_pVert) {
		memcpy(m_pVert,pVertex,m_nBufferSize);
	}

	return true;
}

bool RVertexBuffer::UpdateDataHW(char* pVertex)
{
	if(m_bUseHWVertex && m_vb) {

		Lock();

		memcpy( m_v, pVertex, m_nBufferSize );

		Unlock();
	}

	return true;
}

bool RVertexBuffer::UpdateData(char* pVertex)
{
	UpdateDataHW(pVertex);
	UpdateDataSW(pVertex);

	return true;
}

#ifndef _MAX_EXPORT // max 와 겹침..

void RVertexBuffer::UpdateDataLVert(RLVertex* pVert,D3DXVECTOR3* pVec,int nCnt)
{
	for(int i=0;i<nCnt;i++) {
		pVert[i].p = pVec[i];
	}
}

void RVertexBuffer::UpdateDataVert(RVertex* pVert,D3DXVECTOR3* pVec,int nCnt)
{
	for(int i=0;i<nCnt;i++) {
		pVert[i].p = pVec[i];
	}
}

#endif

// 범용처리는 아닌..

bool RVertexBuffer::UpdateData(D3DXVECTOR3* pVec)
{
	if(!m_is_init) 	return false;

	int nVertType=0;

		 if(m_nVertexSize==sizeof(RLVertex))	{ nVertType = 0; }
	else if(m_nVertexSize==sizeof(RVertex))		{ nVertType = 1; }
//	else if(m_nVertexSize==sizeof(RBlendVertex)){ nVertType = 2; }
	else 	return false;

	if(m_vb) {

		Lock();

		if(nVertType)	UpdateDataVert((RVertex*)m_v,pVec,m_nVertexCnt);
		else			UpdateDataLVert((RLVertex*)m_v,pVec,m_nVertexCnt);

		Unlock();
	}

	if(m_bUseSWVertex && m_pVert) {

		if(nVertType)	UpdateDataVert((RVertex*)m_pVert,pVec,m_nVertexCnt);
		else			UpdateDataLVert((RLVertex*)m_pVert,pVec,m_nVertexCnt);
	}

	return true;
}

bool RVertexBuffer::Update(char* pVertex,DWORD fvf,int VertexSize,int VertexCnt) 
{
	if(!m_is_init) 	return false;

	if(m_dwFVF != fvf) 
		return false;

	if( m_nVertexSize != VertexSize)
		return false;

	int BufferSize = VertexSize * VertexCnt;

	if(m_nBufferSize != BufferSize) {
		if(m_nRealBufferSize < BufferSize) {// 진짜 버퍼보다 작다면 그냥 쓰자..
			if(Create(pVertex,fvf,VertexSize,VertexCnt,m_dwUsage,m_dwPool)==false)
				return false;
		}
		else 
		{
			m_nBufferSize = BufferSize; // m_nRealBufferSize 는 따로다..
			m_nVertexCnt = VertexCnt;
		}
	}

	return UpdateData( pVertex );
}

void RVertexBuffer::Lock() {
	if(m_vb) m_vb->Lock( 0, 0, (VOID**)&m_v, m_dwLockFlag );
//	else 	 assert(0);
}

void RVertexBuffer::Unlock() {
	if(m_vb) m_vb->Unlock();
//	else	 assert(0);
}

void RVertexBuffer::SetStreamSource()
{
	RGetDevice()->SetStreamSource(0,m_vb,0,m_nVertexSize);
}

void RVertexBuffer::Render() {

	if(!m_is_init) 	return;

	LPDIRECT3DDEVICE9 dev = RGetDevice();
	
	if(dev==NULL) return;

	dev->SetStreamSource( 0, m_vb, 0,m_nVertexSize );
	dev->DrawPrimitive( m_PrimitiveType, 0, m_nVertexCnt/3);
}

void RVertexBuffer::RenderFVF()
{
	RGetDevice()->SetFVF( m_dwFVF );
	Render();
}

void RVertexBuffer::RenderSoft()
{
	if(!m_is_init) 	return;

	LPDIRECT3DDEVICE9 dev = RGetDevice();

	if(dev==NULL) return;

	dev->SetFVF( m_dwFVF );
//	dev->SetStreamSource( 0, m_vb, 0,m_nVertexSize );
//	dev->DrawPrimitive( m_PrimitiveType, 0, m_nRenderCnt);
	dev->DrawPrimitiveUP(m_PrimitiveType, m_nVertexCnt/3, (LPVOID) m_pVert, m_nVertexSize);
}

// soft 버퍼만 지원한다..  Vertex Type 인경우..lvert blendvert
// 우선테스트해본다..괜찮으면 버퍼를 만들던가 한다..
void RVertexBuffer::ConvertSilhouetteBuffer(float fLineWidth)
{
	if(!m_pVert) return;

	// normal 이 없으면 의미 없다..

	if(m_nVertexSize==sizeof(RVertex))	{ 

		RVertex* pV = (RVertex*)m_pVert;

		for(int i=0;i<m_nVertexCnt;i++) {
			pV[i].p = pV[i].p + pV[i].n * fLineWidth;
		}
	}
	else if(m_nVertexSize==sizeof(RBlendVertex)){ 

		RBlendVertex* pV = (RBlendVertex*)m_pVert;

		for(int i=0;i<m_nVertexCnt;i++) {
			pV[i].p = pV[i].p + pV[i].normal * fLineWidth;
		}
	}
}

void RVertexBuffer::ReConvertSilhouetteBuffer(float fLineWidth)
{
	if(!m_pVert) return;

	// normal 이 없으면 의미 없다..

	if(m_nVertexSize==sizeof(RVertex))	{ 

		RVertex* pV = (RVertex*)m_pVert;

		for(int i=0;i<m_nVertexCnt;i++) {
			pV[i].p = pV[i].p - pV[i].n * fLineWidth;
		}
	}
	else if(m_nVertexSize==sizeof(RBlendVertex)){ 

		RBlendVertex* pV = (RBlendVertex*)m_pVert;

		for(int i=0;i<m_nVertexCnt;i++) {
			pV[i].p = pV[i].p - pV[i].normal * fLineWidth;
		}
	}
}

void RVertexBuffer::RenderIndexSoft(RIndexBuffer* ib)
{
	if(!m_is_init) 	return;

	LPDIRECT3DDEVICE9 dev = RGetDevice();

	if(dev==NULL) return;

	dev->SetFVF( m_dwFVF );

	dev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,
		m_nVertexCnt,
		ib->GetFaceCnt(),
		ib->m_pIndex,
		D3DFMT_INDEX16,
		(LPVOID) m_pVert,m_nVertexSize);
}

void RVertexBuffer::Render(RIndexBuffer* ib )
{
	if(!m_is_init) 	return;
	if(!ib)			return;

	LPDIRECT3DDEVICE9 dev = RGetDevice();

	if(dev==NULL) return;

	dev->SetStreamSource( 0, m_vb, 0, m_nVertexSize );
	dev->SetIndices(ib->m_ib);
	dev->DrawIndexedPrimitive(m_PrimitiveType,0, 0,m_nVertexCnt,0,ib->GetFaceCnt() );
}

void RVertexBuffer::SetVertexBuffer()
{
	LPDIRECT3DDEVICE9 dev = RGetDevice();
	dev->SetFVF( m_dwFVF );
	dev->SetStreamSource( 0, m_vb, 0, m_nVertexSize );
}

void RVertexBuffer::SetVSVertexBuffer()
{
	LPDIRECT3DDEVICE9 dev = RGetDevice();
	dev->SetStreamSource( 0, m_vb, 0, m_nVertexSize );
}

void RVertexBuffer::RenderIndexBuffer(RIndexBuffer* ib)
{
	if(!ib)			return;
	if(!ib->m_size) return;

	LPDIRECT3DDEVICE9 dev = RGetDevice();

	dev->SetIndices(ib->m_ib);
	dev->DrawIndexedPrimitive(m_PrimitiveType,0, 0,m_nVertexCnt,0,ib->GetFaceCnt() );
}

void GetPath(const char* str,char* path)
{
	if(!str) { 
		path[0] = NULL;
		return;
	}

	int i=0;

	for(i=strlen(str)-1; i>=0; i--) {
		if( (str[i]=='\\') || (str[i]=='/')) {
			strncpy(path,str,i+1);
			path[i+1] = NULL;
			return;
		}
	}
}

static RLVertex t_grid_vert[1000];

//<---------------------------------------------------------------------

void _GetModelTry(RLVertex* pVert,int size,DWORD color,int* face_num)
{
	RLVertex _vert[4];

	RLVertex* t_vert = &_vert[0];

	t_vert->p.x		= 0.f*size;
	t_vert->p.y		= 0.f*size;
	t_vert->p.z		= 0.5f*size;
	t_vert->color	= color;
	t_vert++;

	t_vert->p.x		=-0.5f*size;
	t_vert->p.y		= 0.f*size;
	t_vert->p.z		=-0.5f*size;
	t_vert->color	= color;
	t_vert++;

	t_vert->p.x		= 0.5f*size;
	t_vert->p.y		= 0.f*size;
	t_vert->p.z		=-0.5f*size;
	t_vert->color	= color;
	t_vert++;

	t_vert->p.x		= 0.f*size;
	t_vert->p.y		= 1.5f*size;
	t_vert->p.z		= 0.f*size;
	t_vert->color	= color;

	pVert[0] = _vert[0];
	pVert[1] = _vert[1];
	pVert[2] = _vert[2];

	pVert[3] = _vert[0];
	pVert[4] = _vert[1];
	pVert[5] = _vert[3];

	pVert[6] = _vert[1];
	pVert[7] = _vert[2];
	pVert[8] = _vert[3];

	pVert[9]  = _vert[3];
	pVert[10] = _vert[2];
	pVert[11] = _vert[0];

	// face index num

	*face_num = 4;
}

void _draw_try(LPDIRECT3DDEVICE9 dev,rmatrix& mat,float size,DWORD color)
{
	int face_num = 0;

	_GetModelTry(t_grid_vert,size,color,&face_num);

	dev->SetTransform( D3DTS_WORLD, &mat );
	dev->SetTexture(0, NULL);
	dev->SetFVF( RLVertexType );

	dev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, face_num, (LPVOID) t_grid_vert, sizeof(RLVertex));
}

void _draw_matrix(LPDIRECT3DDEVICE9 dev,rmatrix& mat,float size)
{
	dev->SetRenderState( D3DRS_LIGHTING, FALSE );
	
	rvector pos   = rvector(0,0,0);
	rvector right = rvector(1,0,0)*size;
	rvector up	  = rvector(0,1,0)*size;
	rvector dir   = rvector(0,0,1)*size;

	dev->SetTransform( D3DTS_WORLD, &mat );
	dev->SetTexture(0, NULL);
	dev->SetFVF( RLVertexType );

	RLVertex* t_vert = &t_grid_vert[0];

	////////////////////////////////////

	t_vert->p.x		= pos.x;
	t_vert->p.y		= pos.y;
	t_vert->p.z		= pos.z;
	t_vert->color	= 0xff00ff00;

	t_vert++;

	t_vert->p.x		= right.x;
	t_vert->p.y		= right.y;
	t_vert->p.z		= right.z;
	t_vert->color	= 0xff00ff00;

	t_vert++;

	////////////////////////////////////

	t_vert->p.x		= pos.x;
	t_vert->p.y		= pos.y;
	t_vert->p.z		= pos.z;
	t_vert->color	= 0xffff0000;

	t_vert++;

	t_vert->p.x		= up.x;
	t_vert->p.y		= up.y;
	t_vert->p.z		= up.z;
	t_vert->color	= 0xffff0000;

	t_vert++;

	////////////////////////////////////

	t_vert->p.x		= pos.x;
	t_vert->p.y		= pos.y;
	t_vert->p.z		= pos.z;
	t_vert->color	= 0xff0000ff;

	t_vert++;

	t_vert->p.x		= dir.x;
	t_vert->p.y		= dir.y;
	t_vert->p.z		= dir.z;
	t_vert->color	= 0xff0000ff;

	t_vert++;

	////////////////////////////////////

	dev->DrawPrimitiveUP(D3DPT_LINELIST, 3, (LPVOID) t_grid_vert, sizeof(RLVertex));

	rmatrix _mat = mat;
	rmatrix _tposmat;
	rmatrix _trotmat;

	//right
	D3DXMatrixTranslation(&_tposmat,right.x,right.y,right.z);
	D3DXMatrixRotationZ(&_trotmat,-3.14/2.f);
	_mat = _trotmat*_tposmat*_mat;
	_draw_try(dev,_mat,size/10,0xff00ff00);

	//up
	_mat = mat;
	D3DXMatrixTranslation(&_tposmat,up.x,up.y,up.z);
	_mat = _tposmat*_mat;
	_draw_try(dev,_mat,size/10,0xffff0000);

	//dir
	_mat = mat;
	D3DXMatrixTranslation(&_tposmat,dir.x,dir.y,dir.z);
	D3DXMatrixRotationX(&_trotmat,3.14/2.f);
	_mat = _trotmat*_tposmat*_mat;
	_draw_try(dev,_mat,size/10,0xff0000ff);

	dev->SetRenderState( D3DRS_LIGHTING, TRUE );
}


_USING_NAMESPACE_REALSPACE2
/*
template<typename T>
void RVbuffer<T>::Render()
{
	if(!m_is_init) 	return;

	LPDIRECT3DDEVICE9 dev = RGetDevice();

	if(dev==NULL) return;

	dev->SetStreamSource( 0, m_vb, 0, sizeof(T) );
	dev->SetFVF( m_dwFVF );
	dev->DrawPrimitive( D3DPT_TRIANGLELIST, 0, m_nVertexSize);
	dev->SetStreamSource( 0, NULL, 0, 0 );
}
*/

////////////////////////////////////////////////////////
//


void RRot2Quat( RQuatKey& q,RRotKey& v )
{
	D3DXQUATERNION out;
	D3DXVECTOR3 vec;

	vec.x = v.x;	
	vec.y = v.y;	
	vec.z = v.z;

	D3DXQuaternionRotationAxis(&out,&vec,v.w);

	q.x = out.x;	q.y = out.y;	q.z = out.z;	q.w = out.w;
}

void RQuat2Mat( D3DXMATRIX& mat, RQuatKey&q )
{
	D3DXQUATERNION out;

	out.x = q.x;	
	out.y = q.y;	
	out.z = q.z;	
	out.w = q.w;

	D3DXMatrixRotationQuaternion(&mat,&out);
}

int RMatInv( D3DXMATRIX& q, D3DXMATRIX& a ) {
	D3DXMatrixInverse(&q,NULL,&a);
	return 1;
}

void ConvertMat(rmatrix& mat1,rmatrix& mat2)
{
	mat1 = mat2;

	mat1._11 = mat2._21;
	mat1._12 = mat2._22;
	mat1._13 = mat2._23;

	mat1._21 = mat2._11;
	mat1._22 = mat2._12;
	mat1._23 = mat2._13;

}

void draw_line(LPDIRECT3DDEVICE9 dev,D3DXVECTOR3* vec,int size,DWORD color)
{
	static RLVertex t_vert[50];

	for(int i=0;i<size;i++) {
		t_vert[i].p.x = vec[i].x;
		t_vert[i].p.y = vec[i].y;
		t_vert[i].p.z = vec[i].z;
		t_vert[i].color = color;
	}

	dev->SetTexture(0, NULL);
	dev->SetFVF( RLVertexType );

	dev->DrawPrimitiveUP(D3DPT_LINELIST, size, (LPVOID) t_vert, sizeof(RLVertex));
}

struct	_Vertex { 
	float x,y,z;
//	D3DXVECTOR3 p;   
	DWORD color;	// color 도 제거?
};
#define _VertexType			(D3DFVF_XYZ | D3DFVF_DIFFUSE)

void draw_query_fill_box(rmatrix* wmat , rvector& max,rvector& min,DWORD color)
{
	// 컬러 채우기 끄고..
	// z write 끄고..
	// flat shade
	// light false
	// texture Null .. 어지간한건 모두 끄고..
/*
	   5 ----   6
	   /    / |
	1  ----	2 |
	  |	   |  | 7
	  |	   | /
	0  ----	3
	
*/

//	D3DRS_ANTIALIASEDLINEENABLE
//  D3DRS_MULTISAMPLEANTIALIAS 안티알리어스.. 점을 찍을때 주변점과의 관계를 생각해서 픽셀의 색을 변경하는것.. 간단함..	타겟버퍼에 플레그 설정함,../

//	D3DRS_COLORVERTEX
//	D3DRS_DIFFUSEMATERIALSOURCE
//	D3DRS_COLORWRITEENABLE		   D3DCAPS9 구조체의 PrimitiveMiscCaps 멤버로 D3DPMISCCAPS_COLORWRITEENABLE 능력 비트가 설정되어 있는 경우에 이용할 수 있다. 이 렌더링 스테이트는, 클리어 처리에는 영향을 주지 않는다

	static _Vertex _vert[8];

	_vert[0].x = min.x;	_vert[0].y = min.y;	_vert[0].z = min.z; _vert[0].color = color;
	_vert[1].x = min.x;	_vert[1].y = max.y;	_vert[1].z = min.z; _vert[1].color = color;
	_vert[2].x = max.x;	_vert[2].y = max.y;	_vert[2].z = min.z; _vert[2].color = color;
	_vert[3].x = max.x;	_vert[3].y = min.y;	_vert[3].z = min.z;	_vert[3].color = color;
	_vert[4].x = min.x;	_vert[4].y = min.y;	_vert[4].z = max.z;	_vert[4].color = color;
	_vert[5].x = min.x;	_vert[5].y = max.y;	_vert[5].z = max.z;	_vert[5].color = color;
	_vert[6].x = max.x;	_vert[6].y = max.y;	_vert[6].z = max.z;	_vert[6].color = color;
	_vert[7].x = max.x;	_vert[7].y = min.y;	_vert[7].z = max.z;	_vert[7].color = color;

	static WORD	_index[36] =  {
		0,1,3 ,
		1,2,3 ,
		3,2,7 ,
		2,6,7 ,
		7,6,4 ,
		6,5,4 ,
		4,5,0 ,
		5,1,0 ,
		1,5,2 ,
		5,6,2 ,
		4,0,7 ,
		0,3,7  
	};

	RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );

//	rmatrix savemat;
	
//	RGetDevice()->GetTransform( D3DTS_WORLD, &savemat );
	RGetDevice()->SetTransform( D3DTS_WORLD, wmat );
	RGetDevice()->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT );
	RGetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	RGetDevice()->SetRenderState( D3DRS_ZWRITEENABLE , FALSE );

	RGetDevice()->SetTexture( 0,NULL );
	RGetDevice()->SetFVF( _VertexType );

	RGetDevice()->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,
		8, 12, _index, D3DFMT_INDEX16, (LPVOID)_vert,sizeof(_Vertex));

//	RGetDevice()->SetTransform( D3DTS_WORLD, &savemat );
//	RGetDevice()->SetRenderState( D3DRS_LIGHTING, TRUE );
	RGetDevice()->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
	RGetDevice()->SetRenderState( D3DRS_ZWRITEENABLE , TRUE );
}

void draw_box(rmatrix* wmat , rvector& max,rvector& min,DWORD color)
{
	RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );

	rvector t_vec_d[24];
//	int		t_index[100];	//인덱스 방식으로 나중에 개량
	rvector t_vec[8];

	t_vec[0].x = min.x;	t_vec[0].y = min.y;	t_vec[0].z = min.z;
	t_vec[1].x = min.x;	t_vec[1].y = max.y;	t_vec[1].z = min.z;
	t_vec[2].x = max.x;	t_vec[2].y = max.y;	t_vec[2].z = min.z;
	t_vec[3].x = max.x;	t_vec[3].y = min.y;	t_vec[3].z = min.z;
	t_vec[4].x = min.x;	t_vec[4].y = min.y;	t_vec[4].z = max.z;
	t_vec[5].x = min.x;	t_vec[5].y = max.y;	t_vec[5].z = max.z;
	t_vec[6].x = max.x;	t_vec[6].y = max.y;	t_vec[6].z = max.z;
	t_vec[7].x = max.x;	t_vec[7].y = min.y;	t_vec[7].z = max.z;

	t_vec_d[0]  = t_vec[0];	t_vec_d[1]  = t_vec[1];
	t_vec_d[2]  = t_vec[1];	t_vec_d[3]  = t_vec[2];
	t_vec_d[4]  = t_vec[2];	t_vec_d[5]  = t_vec[3];
	t_vec_d[6]  = t_vec[3];	t_vec_d[7]  = t_vec[0];

	t_vec_d[8]  = t_vec[1];	t_vec_d[9]  = t_vec[5];
	t_vec_d[10] = t_vec[2];	t_vec_d[11] = t_vec[6];
	t_vec_d[12] = t_vec[0];	t_vec_d[13] = t_vec[4];
	t_vec_d[14] = t_vec[3];	t_vec_d[15] = t_vec[7];

	t_vec_d[16] = t_vec[4];	t_vec_d[17] = t_vec[5];
	t_vec_d[18] = t_vec[5];	t_vec_d[19] = t_vec[6];
	t_vec_d[20] = t_vec[6];	t_vec_d[21] = t_vec[7];
	t_vec_d[22] = t_vec[7];	t_vec_d[23] = t_vec[4];

	// render mode	처리
	// pos			처리
/*
	if(wmat) {
		for(int i=0;i<24;i++) {
//			t_vec_d[i] = t_vec_d[i] * (*wmat);
			D3DXVec3TransformCoord(&t_vec_d[i],&t_vec_d[i],wmat);
		}
	}
*/
	rmatrix savemat;

	RGetDevice()->GetTransform( D3DTS_WORLD, &savemat );

	RGetDevice()->SetTransform( D3DTS_WORLD, wmat );

	draw_line(RGetDevice(),t_vec_d,24,color);

	RGetDevice()->SetTransform( D3DTS_WORLD, &savemat );

	RGetDevice()->SetRenderState( D3DRS_LIGHTING, TRUE );

}

CD3DArcBall::CD3DArcBall()
{
	D3DXQuaternionIdentity( &m_qDown );
	D3DXQuaternionIdentity( &m_qNow );
	D3DXMatrixIdentity( &m_matRotation );
	D3DXMatrixIdentity( &m_matRotationDelta );
	D3DXMatrixIdentity( &m_matTranslation );
	D3DXMatrixIdentity( &m_matTranslationDelta );
	m_bDrag = FALSE;
	m_fRadiusTranslation = 1.0f;
	m_bRightHanded = FALSE;
}

VOID CD3DArcBall::SetWindow( int iWidth, int iHeight, float fRadius )
{
	m_iWidth  = iWidth;
	m_iHeight = iHeight;
	m_fRadius = fRadius;
}

D3DXVECTOR3 CD3DArcBall::ScreenToVector( int sx, int sy )
{
	FLOAT x   = -(sx - m_iWidth /2) / (m_fRadius*m_iWidth /2);
	FLOAT y   =  (sy - m_iHeight/2) / (m_fRadius*m_iHeight/2);

	if( m_bRightHanded ) {
		x = -x;
		y = -y;
	}

	FLOAT z   = 0.0f;
	FLOAT mag = x*x + y*y;

	if( mag > 1.0f ) {
		FLOAT scale = 1.0f/sqrtf(mag);
		x *= scale;
		y *= scale;
	}
	else
		z = sqrtf( 1.0f - mag );

	return D3DXVECTOR3( x, y, z );
}

VOID CD3DArcBall::SetRadius( FLOAT fRadius )
{
	m_fRadiusTranslation = fRadius;
}

LRESULT CD3DArcBall::HandleMouseMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	static int         iCurMouseX;      
	static int         iCurMouseY;

	static D3DXVECTOR3 s_vDown;         

	int iMouseX = LOWORD(lParam);
	int iMouseY = HIWORD(lParam);

	switch( uMsg )
	{
	case WM_MBUTTONDOWN:
		iCurMouseX = iMouseX;
		iCurMouseY = iMouseY;
		return TRUE;

	case WM_RBUTTONDOWN:
		m_bDrag = TRUE;
		s_vDown = ScreenToVector( iMouseX, iMouseY );
		m_qDown = m_qNow;
		return TRUE;

	case WM_RBUTTONUP:
		m_bDrag = FALSE;
		return TRUE;

	case WM_MOUSEMOVE:
		if( MK_RBUTTON & wParam )  {
			if( m_bDrag )  {
				D3DXVECTOR3 vCur = ScreenToVector( iMouseX, iMouseY );
				D3DXQUATERNION qAxisToAxis;
				D3DXQuaternionAxisToAxis(&qAxisToAxis, &s_vDown, &vCur);
				m_qNow = m_qDown;
				m_qNow *= qAxisToAxis;
				D3DXMatrixRotationQuaternion(&m_matRotationDelta, &qAxisToAxis);
			}
			else
				D3DXMatrixIdentity(&m_matRotationDelta);

			D3DXMatrixRotationQuaternion(&m_matRotation, &m_qNow);
			m_bDrag = TRUE;
		}
		return TRUE;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////

RDebugStr::RDebugStr()
{

}
RDebugStr::~RDebugStr()
{

}

void RDebugStr::Clear() {
	m_str.clear();
}

void RDebugStr::Add(char* str,bool line) {

	if(!str) return;

	m_str += str;

	if(line) AddLine();
}

void RDebugStr::Add(bool b,bool line) {

	if(b) 
		sprintf_safe(m_temp,"true");
	else 
		sprintf_safe(m_temp,"false");

	m_str += m_temp;

	if(line) AddLine();
}

void RDebugStr::Add(char c,bool line) {

	sprintf_safe(m_temp,"%c",c);
	m_str += m_temp;

	if(line) AddLine();
}

void RDebugStr::Add(short s,bool line) {

	sprintf_safe(m_temp,"%d",s);
	m_str += m_temp;

	if(line) AddLine();
}

void RDebugStr::Add(WORD w,bool line) {

	sprintf_safe(m_temp,"%d",w);
	m_str += m_temp;

	if(line) AddLine();
}

void RDebugStr::Add(int i,bool line) {

	sprintf_safe(m_temp,"%d",i);
	m_str += m_temp;

	if(line) AddLine();
}

void RDebugStr::Add(DWORD d,bool line) {

	sprintf_safe(m_temp,"%d",d);
	m_str += m_temp;

	if(line) AddLine();
}

void RDebugStr::Add(float f,bool line) {

	sprintf_safe(m_temp,"%f",f);
	m_str += m_temp;

	if(line) AddLine();
}

void RDebugStr::Add(rvector& v,bool line) {

	sprintf_safe(m_temp,"%f %f %f", v.x, v.y, v.z);
	m_str += m_temp;

	if(line) AddLine();
}

void RDebugStr::AddLine(int cnt) {

	for(int i=0;i<cnt;i++) {
		sprintf_safe(m_temp,"\n");
		m_str += m_temp;
	}
}

void RDebugStr::AddTab(int cnt) {
	for(int i=0;i<cnt;i++){
		sprintf_safe(m_temp,"\t");
		m_str += m_temp;
	}
}

void RDebugStr::PrintLog() {
	mlog( m_str.c_str() );
}

////////////////////////////////////////////////////////////////////////

char* RBaseObject::GetName()
{
	return (char*)m_Name.c_str();
}

void RBaseObject::SetName(char* name)
{
	m_Name = name;
}

bool RBaseObject::CheckName(string& name)
{
	if(m_Name==name)
		return true;
	return false;
}

bool RBaseObject::CheckName(char* name) 
{
	return CheckName(string(name));
}

//////////////////////////////////////////////////////////////////////////////////////////
