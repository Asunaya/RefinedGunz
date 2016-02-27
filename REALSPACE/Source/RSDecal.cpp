// RSDecal.cpp: implementation of the RSDecal class.
//
//////////////////////////////////////////////////////////////////////

#include "RealSpace.h"
#include "RSDecal.h"
#include "RSDebug.h"

//#define EL_TEST

RSDecal::RSDecal()
{
	v[0].x=-1;	v[0].y=-1;	v[0].z=0;	v[0].tu1=0;	v[0].tv1=0;
	v[1].x=1;	v[1].y=-1;	v[1].z=0;	v[1].tu1=1;	v[1].tv1=0;
	v[2].x=-1;	v[2].y=1;	v[2].z=0;	v[2].tu1=0;	v[2].tv1=1;
	v[3].x=1;	v[3].y=1;	v[3].z=0;	v[3].tu1=1;	v[3].tv1=1;
	
	TextureHandle=NULL;

	v[0].Diffuse=v[1].Diffuse=v[2].Diffuse=v[3].Diffuse=0xffffffff;
	v[0].Specular=v[1].Specular=v[2].Specular=v[3].Specular=0xff000000;

	mat=IdentityMatrix();
	CorMat=IdentityMatrix();
 
	ScaleFactorY=ScaleFactorX=1.0f;
	Material=NULL;
}

RSDecal::~RSDecal()
{
}

#ifdef _DEBUG
// temp 
int g_decal_call_cnt;
int g_decal_call_cnt2;
int g_decal_call_cnt3;

#endif

inline LITVERTEX operator*( LITVERTEX &in_vec,rmatrix44 &mat)
{
	LITVERTEX out;

	FLOAT x = in_vec.x*mat._11 + in_vec.y*mat._21 + in_vec.z* mat._31 + mat._41;
	FLOAT y = in_vec.x*mat._12 + in_vec.y*mat._22 + in_vec.z* mat._32 + mat._42;
	FLOAT z = in_vec.x*mat._13 + in_vec.y*mat._23 + in_vec.z* mat._33 + mat._43;
	FLOAT w = in_vec.x*mat._14 + in_vec.y*mat._24 + in_vec.z* mat._34 + mat._44;

	out.x = x/w;
	out.y = y/w;
	out.z = z/w;

	return out;
}

void RSDecal::Draw()
{
#ifndef EL_TEST

	rmatrix44 tm=CorMat*mat*TranslateMatrix44(position);
	g_pd3dDevice->SetTransform(D3DTS_WORLD, (_D3DMATRIX*)&tm);
	g_pd3dDevice->SetVertexShader(RSLFVF);
	RSSetTexture(TextureHandle?TextureHandle:Material?Material->TextureHandle:0);
	HRESULT result=g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,v,sizeof(LITVERTEX));
	_ASSERT(result==D3D_OK);

#else

	static LITVERTEX lv[6];

	rmatrix44 tm=CorMat*mat*TranslateMatrix44(position);
	int tex_id = TextureHandle?TextureHandle:Material?Material->TextureHandle:0;

	lv[0] = v[0] * tm;
	lv[1] = v[1] * tm;
	lv[2] = v[2] * tm;
	lv[3] = v[1] * tm;
	lv[4] = v[3] * tm;
	lv[5] = v[2] * tm;

	// 이것두 한번에 하도록 수정
	RSDrawPoly(lv	,tex_id);
	RSDrawPoly(lv+3 ,tex_id);

#endif

#ifdef _DEBUG
	g_decal_call_cnt++;
#endif

}

void RSDecal::Draw(rmatrix44 &tm)
{
	int i;
	TRANSLITVERTEX ver[4];
	for(i=0;i<4;i++)
	{
		*(rvector*)(&ver[i].x)=TransformVector(*(rvector*)&v[i].x,tm);
		ver[i].rhw=1.0f;
		ver[i].tu1=v[i].tu1;
		ver[i].tv1=v[i].tv1;
		ver[i].Diffuse=v[i].Diffuse;
	}

	g_pd3dDevice->SetVertexShader(RSTLFVF);
	RSSetTexture(TextureHandle?TextureHandle:Material?Material->TextureHandle:0);
	g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,ver,sizeof(TRANSLITVERTEX));

#ifdef _DEBUG
	g_decal_call_cnt2++;
#endif

}


void RSDecal::SetScaleXY(float sx,float sy)
{
	ScaleFactorX=sx;ScaleFactorY=sy;
	CorMat=ScaleMatrixXYZ(sx,sy,1.0f);
}

void RSDecal::AddRotationX(float theta)
{
	mat=MatrixMult(RotateXMatrix(theta),mat);
}

void RSDecal::AddRotationY(float theta)
{
	mat=MatrixMult(RotateYMatrix(theta),mat);
}

void RSDecal::AddRotationZ(float theta)
{
	mat=MatrixMult(RotateZMatrix(theta),mat);
}

void RSDecal::SetOrientation(rvector dir, rvector up)
{
	rvector right;

	dir=-Normalize(dir);
	right=Normalize(CrossProduct(dir,-up));
	up=-Normalize(CrossProduct(dir,right));
	mat._11=right.x;mat._12=right.y;mat._13=right.z;
	mat._21=dir.x;mat._22=dir.y;mat._23=dir.z;
	mat._31=up.x;mat._32=up.y;mat._33=up.z;
}

void RSDecal::SetOrientationToCamera(float role)
{
	rvector dir,up,right;
	dir.x=RSViewProjection._12;dir.y=RSViewProjection._22;dir.z=RSViewProjection._32;
	right = CrossProduct(rvector(0,0,1), dir);
	if(IS_EQ(right.GetMagnitude(),0)) right=rvector(1,0,0);
		else right = Normalize(right);
	up = Normalize(CrossProduct(dir, right));
	SetOrientation(dir,up);
	mat=MatrixMult(RotateZMatrix(role),mat);
}

void RSDecal::GetOrientation(rvector *dir, rvector *up)
{
	*up=rvector(mat._13,mat._23,mat._33);
	*dir=rvector(mat._12,-mat._22,mat._32);
}
		
void RSDecal::SetColor(DWORD cl)
{
	v[0].Diffuse=v[1].Diffuse=v[2].Diffuse=v[3].Diffuse=cl;
}

void RSDecal::SetMaterial(RSMaterial *material)
{	
	Material=material;
	thistime=0;
}

RSMaterial* RSDecal::GetMaterial()
{
	return Material;
}

void RSDecal::SetTexture(int iT)
{
	TextureHandle=iT;
}

int	RSDecal::GetTextureHandle()
{
	return TextureHandle;
}

void RSDecal::InitAnimation(void)
{
	thistime = 0;
}

void RSDecal::MoveAnimation(DWORD time)
{
	thistime+=time;
	TextureHandle=Material->TextureHandles[
		(thistime/Material->AnimationSpeed)%Material->nTexture];
}

int RSDecal::GetAnimationCount(void)
{
	return Material?Material->nTexture:1;
}

int RSDecal::GetAnimationSpeed(void)
{
	return Material?Material->AnimationSpeed:1;
}

void RSDecal::MoveAminmationByIndex(int i)
{
	if(Material!=NULL && Material->TextureHandles!=NULL)
		TextureHandle=Material->TextureHandles[ i % Material->nTexture ];
}

int RSDecal::GetTextureHandle(int i)
{
	if(Material!=NULL && Material->TextureHandles!=NULL)
		return Material->TextureHandles[ i % Material->nTexture ];
	return 0;
}

void RSDecal::DrawPanel( int nLeft, int nTop, int nRight, int nBottom)
{
	/*
	static rface fc[2];
	static rvertex vx[4];

	vx[0].sx = (float) nLeft;	vx[0].sy = (float) nBottom;
	vx[1].sx = (float) nRight;	vx[1].sy = (float) nBottom;
	vx[2].sx = (float) nRight;	vx[2].sy = (float) nTop;
	vx[3].sx = (float) nLeft;	vx[3].sy = (float) nTop;

	fc[0].v[0]=vx; fc[0].v[1]=vx+1;	fc[0].v[2]=vx+2;
	fc[1].v[0]=vx; fc[1].v[1]=vx+2; fc[1].v[2]=vx+3;

	fc[0].s[0] = 0;	fc[0].t[0] = 1;
	fc[0].s[1] = 1;	fc[0].t[1] = 1;
	fc[0].s[2] = 1;	fc[0].t[2] = 0;

	fc[1].s[0] = 0;	fc[1].t[0] = 1;
	fc[1].s[1] = 1;	fc[1].t[1] = 0;
	fc[1].s[2] = 0;	fc[1].t[2] = 0;

	vx[0].sz = vx[1].sz = vx[2].sz = vx[3].sz = vx[0].z = vx[1].z = vx[2].z = vx[3].z = 0;

	vx[0].pw = 0.1f;
	vx[1].pw = 0.1f;
	vx[2].pw = 0.1f;
	vx[3].pw = 0.1f;

	vx[0].color=v[0].Diffuse;vx[1].color=v[1].Diffuse;vx[2].color=v[2].Diffuse;vx[3].color=v[3].Diffuse;

	g_pd3dDevice->SetVertexShader(RSTLFVF);
	RSSetTexture(TextureHandle?TextureHandle:Material?Material->TextureHandle:0);
	RSDrawFace(&fc[0]);
	RSDrawFace(&fc[1]);
	*/

	static TRANSLITVERTEX ver[4]= {	{0,0,0,1,0xffffffff,0,0,0 },
									{0,0,0,1,0xffffffff,0,1,0 },
									{0,0,0,1,0xffffffff,0,0,1 },
									{0,0,0,1,0xffffffff,0,1,1 }};
	ver[0].x=(float)nLeft;ver[0].y=(float)nTop;
	ver[1].x=(float)nRight;ver[1].y=(float)nTop;
	ver[2].x=(float)nLeft;ver[2].y=(float)nBottom;
	ver[3].x=(float)nRight;ver[3].y=(float)nBottom;
	ver[0].Diffuse=ver[1].Diffuse=ver[2].Diffuse=ver[3].Diffuse=v[0].Diffuse;
	
	g_pd3dDevice->SetVertexShader(RSTLFVF);
	RSSetTexture(TextureHandle?TextureHandle:Material?Material->TextureHandle:0);
	g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,ver,sizeof(TRANSLITVERTEX));

#ifdef _DEBUG
	g_decal_call_cnt3+=2;
#endif
}

void RSDecal::SetOpacity( unsigned char nOpacity )
{
	v[0].Diffuse = RSCOMBINEALPHA(v[0].Diffuse, nOpacity);
	v[1].Diffuse = RSCOMBINEALPHA(v[1].Diffuse, nOpacity);
	v[2].Diffuse = RSCOMBINEALPHA(v[2].Diffuse, nOpacity);
	v[3].Diffuse = RSCOMBINEALPHA(v[3].Diffuse, nOpacity);
}