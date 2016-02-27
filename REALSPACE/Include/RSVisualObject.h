// RSVisualObject.h

#ifndef __RSVISUALOBJECT_H
#define __RSVISUALOBJECT_H

#include "rutils.h"
#include "RSObject.h"
#include "RSBase3D.h"
#include "ASSMap.h"
#include "RSAnimationInfo.h"

#define DRAFT_SHAODW_SIZE	1.2f/2.0f

class RSVisualSingleObject;

class RSVisualObject : public RSBase3D 
{
	friend ASSMap;
	friend RealSpace;
	friend RSVisualSingleObject;

public:
	RSVisualObject();
	virtual ~RSVisualObject();

	bool Create(const RSObject *r);

	void MoveAnimation(float elapsedtime);
	void SetAnimationStyle(int a) {AnimationStyle=a;};
	int GetAnimationIndex(char* szName);
	void SetAnimation(int);
	int GetAnimationFrameCount() { return CurrentAnimation->GetFrameCount(); }
	float GetAnimaitonFrameSpeed() { return CurrentAnimation->GetFrameSpeed(); }
	void SetAnimationFrame(float a) { if((a>=0)&&(a<CurrentAnimation->GetFrameCount())) AnimationKey=a; }
	float GetAnimationFrame() { return AnimationKey; }
	RSAnimationInfo* GetCurrentAnimation(){ return CurrentAnimation; }
	int	GetCurrentAnimationIndex() { return Animation; }
	
	bool IsAnimationStopped(){return(((int)AnimationKey)==(CurrentAnimation->GetFrameCount()-1));}
	void SeekFrame( RSANISEEKSTYLE style ){
		if( style == RS_SEEK_END ) AnimationKey = (float)(CurrentAnimation->GetFrameCount() - 1);
		else AnimationKey = (float)style;	}

// about draw.....
	virtual void Transform();
	virtual void TransformAndLight(RSPointLightList *PointLightList);
	virtual void SetupLight(RSPointLightList *PointLightList);
	virtual void Draw();
	virtual void DrawMono(DWORD color);
	virtual void DrawShadow(ASSMap *TargetMap);
	virtual bool DrawReflect(rvector dir,rvector apoint); // draw reflection wrt any plane

	virtual void DrawBoundingBox();					// 디버그를 위한 펑션.
	virtual void DrawNormal(float fLineLength);		//	draw normal especially for debug

// etc....
	bool BoundingBoxTest();
	bool BoundingBoxTest(rplane *planes);
	bool BoundingBoxTest_Shadow(rplane *planes,ASSMap *TargetMap);
	bool BoundingSphereTest(rvector *center,float radius);

	void SetHeight(float y);
	void SetCombineColor(rvector cl);
	void SetFogColor(DWORD dwColor) {m_FogColor=dwColor;}
	float GetBoundingSphereRadius() { return rsm->m_BoundingSphereRadius; }

// environment mapping 에 관한.
	void SetEnvironmentOnly(bool bEnvOnly) { m_bEnvOnly=bEnvOnly; }
	void SetEnvironmentMethod(RSENVIRONMENTSTYLE estyle) { m_EnvStyle=estyle; }
	void SetEnvironmentTextureHandle(int eh) { m_hEnvTextureHandle=eh; }
	void SetEnvironmentBias(float u,float v) { m_fEnvBiasU=u;m_fEnvBiasV=v; }
	void SetEnvironmentColor(DWORD color) { m_dwEnvColor=color; }

// state 들.

	void SetShadowTextureHandle(int sh) { ShadowTextureHandle=sh; } // for draft shadow
	void SetZBufferState(bool r,bool w) { bReadZ=r;bWriteZ=w; }
	void SetAlphaStyle(RSALPHASTYLE a)	{ m_AlphaState=a;		}
	void SetOpacity(unsigned char opacity)	{	m_opacity=opacity;	}
	void SetAlphaState(RSALPHASTYLE a,unsigned char opacity) { SetAlphaStyle(a);SetOpacity(opacity); }
	RSALPHASTYLE GetAlphaStyle()		{ return m_AlphaState; }
	unsigned char GetOpacity()			{ return m_opacity; }
	unsigned char GetFog()				{ return unsigned char(m_FogColor>>24); }

	unsigned char GetCurrentOpacity()	{ return m_bVisible? min(GetOpacity(),GetFog()) : 0; }		// 현재프레임의 opac.

// picking 관련
	bool isContainThisPoint(int x,int y,float *distance);
	bool Pick(const rvector &origin,const rvector &target,rvector *ret);
	// Pick에서 origin과 target은 world 좌표계임. 리턴값은 model 좌표계.
	rvector FindNearestVertex(rvector &position);
	// 가장 가까운 버텍스를 찾는다. position,리턴 모두 모델 좌표계.

// from model coordinate system to ...
	rvector GetRandomPoint();			// 모델좌표계로 아무 점이나 리턴해줌.
	virtual rvector GetScreenCoordinate(rvector&);
	virtual rvector GetWorldCoordinate(rvector&);

// about interface...
	RSObject* GetAttachedRSM(){return rsm;}
	void GetRect(LPFRECT ,float *);
	void ReplaceSubObject(RSObject*);
	bool ReplaceSubObject(const char *TargetName,const RSObject *obj,const char *SourceName);
	int GetSubObjectCount() { return nMesh; }
	CMesh *GetSubObject(int i) { return meshestoview[i]; }
	bool ReplaceMaterial(const char *MaterialName,RSMaterial *pMaterial);

private:
// don't call followings directly.....
	void Draw(int iMesh,CFaces *faces);
	void DrawDraftShadow();
	void DrawFlatShadow(rvector &lightdir);
	void DrawTextureProjectionShadow(ASSMap *TargetMap,rvector &lightdir);
	void DrawDetailTextureProjectionShadow(ASSMap *TargetMap,rvector &lightdir);
	void GetSubObjectTM(rmatrix *tm,rmatrix *model,int i);
	void AlignPoint(rmatrix &tm,rvector &camera);
	void AlignLine(rmatrix &tm,rvector &camera);
	bool GetIsRendered() {return m_isRender; }

// member variables...
	rboundingbox m_BoundingBox;
	rvertex m_BoundingVertices[8];
	float AnimationKey;
	int nAnimSet,Animation,AnimationStyle;
	int	nMesh,nMaterial;

	RSMaterial		**materials;
	RSAnimationInfo	*CurrentAnimation;
	CMesh		**meshestoview;
// for shadow bb test
	rvector		m_vShadowPicked;
	rmatrix		m_ShadowProjectionMatrix;

	DWORD ShadowTextureHandle;
/*	
// for draft shadow
	rvertex shadowver[4];
	rface shadowface[2];
*/
// for environment mapping
	bool	m_bEnvOnly;
	bool	m_isRender;
	RSENVIRONMENTSTYLE	m_EnvStyle;
	DWORD m_hEnvTextureHandle,m_dwEnvColor;
	float m_fEnvBiasU,m_fEnvBiasV;

//  object's attribute
	DWORD m_FogColor;
	unsigned char m_opacity;
	bool bReadZ,bWriteZ;
	RSALPHASTYLE m_AlphaState;
	rvector CombineColor;

	RSObject *rsm;
};

#endif