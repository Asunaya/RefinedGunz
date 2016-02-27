// RSBase3D.cpp : Base Class of 3D Geometrical Object

#include "RSBase3D.h"
#include "rutils.h"
#include "RealSpace.h"

RSBase3D::RSBase3D()
{
	position=rvector(0,0,0);
	m_dir=rvector(0,-1,0);m_up=rvector(0,0,1);
	parent=NULL;
	matm=IdentityMatrix();
	mat=IdentityMatrix();
	MaxScaleFactor=ScaleFactorX=ScaleFactorY=ScaleFactorZ=1.0f;
	RoleFactor=0.0f;LastFrame=0;
	nShadowStyle=RS_SHADOWSTYLE_NONE;
	m_bVisible=true;
}

RSBase3D::~RSBase3D()
{
	m_ChildrenList.DeleteRecordAll();
}


// virtual functions
void RSBase3D::Transform(){}
void RSBase3D::TransformAndLight(RSPointLightList *PointLightList){}
void RSBase3D::SetupLight(RSPointLightList *PointLightList){}
void RSBase3D::Draw(){}
void RSBase3D::DrawShadow(ASSMap *TargetMap){}
void RSBase3D::MoveAnimation(float elapsedtime){}
void RSBase3D::SetFOW(float){}
void RSBase3D::SetFogColor(DWORD dwColor){}
bool RSBase3D::BoundingBoxTest(rplane *planes){return true;}
bool RSBase3D::BoundingBoxTest_Shadow(rplane *planes,ASSMap *TargetMap){return true;}
bool RSBase3D::BoundingSphereTest(rvector *center,float radius){return true;}

// Main Functions
void RSBase3D::AddChild(RSBase3D *child)
{
	m_ChildrenList.Add(child);
	child->parent=this;
	UpdateMaxScaleFactor();
}

rmatrix RSBase3D::GetMatrix()
{
	if(LastFrame!=RSFrameCount)
	{
		matm=MatrixMult(MatrixMult(RotateYMatrix(RoleFactor),
			ScaleMatrixXYZ(ScaleFactorX,ScaleFactorY,ScaleFactorZ)),mat);
		if(parent)matm=MatrixMult(matm,parent->GetMatrix());
		LastFrame=RSFrameCount;
	}
	return matm;
}

void RSBase3D::UpdateMatrix()
{
	static rvector dir,up,right;

	dir=-Normalize(m_dir);
	right=Normalize(CrossProduct(dir,m_up));
	up=Normalize(CrossProduct(right,dir));
	mat._11=right.x;mat._12=right.y;mat._13=right.z;
	mat._21=dir.x;mat._22=dir.y;mat._23=dir.z;
	mat._31=up.x;mat._32=up.y;mat._33=up.z;
	mat._41=position.x;mat._42=position.y;mat._43=position.z;

	matm=MatrixMult(MatrixMult(RotateYMatrix(RoleFactor),
		ScaleMatrixXYZ(ScaleFactorX,ScaleFactorY,ScaleFactorZ)),mat);
	if(parent)matm=MatrixMult(matm,parent->GetMatrix());
	LastFrame=RSFrameCount;

}

void RSBase3D::SetPosition(float x , float y, float z)
{
	position.x=x;position.y=y;position.z=z;
	UpdateMatrix();
}

void RSBase3D::SetOrientation(const rvector &dir, const rvector &up)
{
	m_dir=dir;
	m_up=up;
	UpdateMatrix();
}

bool RSBase3D::IsFog(rvector camera_pos,float r)
{
	rvector pos = GetWorldPosition();

	camera_pos.z = 0;
	pos.z = 0;

	pos = camera_pos - pos;

	if(pos.GetMagnitude() > r) return true;
	return false;
}

rvector RSBase3D::GetWorldPosition()
{
	rmatrix tm=GetMatrix();
	return rvector(tm._41,tm._42,tm._43);
}

void RSBase3D::GetDirectionVectors()
{
	m_up=rvector(mat._13,mat._23,mat._33);
	m_dir=rvector(mat._12,-mat._22,mat._32);
}

void RSBase3D::AddRotationX(float theta)
{
	mat=MatrixMult(RotateXMatrix(theta),mat);
	GetDirectionVectors();
}

void RSBase3D::AddRotationY(float theta)
{
	mat=MatrixMult(RotateYMatrix(theta),mat);
	GetDirectionVectors();
}

void RSBase3D::AddRotationZ(float theta)
{
	mat=MatrixMult(RotateZMatrix(theta),mat);
	GetDirectionVectors();
}


void RSBase3D::SetScale(float s)
{
	ScaleFactorX=ScaleFactorY=ScaleFactorZ=s;
	MaxScaleFactor=s;
	UpdateMatrix();
}

void RSBase3D::SetScale(float x,float y,float z)
{
	ScaleFactorX=x;ScaleFactorY=y;ScaleFactorZ=z;
	MaxScaleFactor=1.0f;
	UpdateMaxScaleFactor();
	UpdateMatrix();
}

void RSBase3D::GetScale(float *x,float *y,float *z)
{
	if(parent)
	{
		float sx,sy,sz;
		parent->GetScale(&sx,&sy,&sz);
		*x=sx*ScaleFactorX;
		*y=sy*ScaleFactorY;
		*z=sz*ScaleFactorZ;
	}
	else
	{
		*x=ScaleFactorX;
		*y=ScaleFactorY;
		*z=ScaleFactorZ;
	}
}

float RSBase3D::GetMaxScaleFactor()
{
	if(parent)
		return MaxScaleFactor*parent->GetMaxScaleFactor();
	else
		return MaxScaleFactor;
}

void RSBase3D::UpdateMaxScaleFactor()
{
	MaxScaleFactor=max(max(ScaleFactorX,ScaleFactorY),ScaleFactorZ);
	for(int i=0;i<m_ChildrenList.GetCount();i++)
		m_ChildrenList.Get(i)->UpdateMaxScaleFactor();
}


void RSBase3D::SetRole(float role)
{
	RoleFactor=role;
	UpdateMatrix();
}

void RSBase3D::GetOrientation(rvector *dir, rvector *up)
{
	*up=rvector(mat._13,mat._23,mat._33);
	*dir=rvector(mat._12,-mat._22,mat._32);
}