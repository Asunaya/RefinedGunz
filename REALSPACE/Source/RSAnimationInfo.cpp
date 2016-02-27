#include "crtdbg.h"
#include "RSAnimationInfo.h"
#include "rutils.h"

RSAnimationInfo::RSAnimationInfo()
{
	name=NULL;
	speed=1.0f;
	tm=NULL;
	nMesh=0;
	nVertAnim=NULL;
	vertanim=NULL;
}

RSAnimationInfo::~RSAnimationInfo()
{
	int i;

	if(name) delete []name;
	if(tm) delete []tm;
	if(nVertAnim) delete []nVertAnim;
	if(vertanim)
	{
		for(i=0;i<nMesh;i++)
			delete [](vertanim[i]);
		delete []vertanim;
	}
	if(Visibility)
		delete []Visibility;
}

float RSAnimationInfo::GetDuration()									// 애니메이션 한바퀴 도는데 걸리는 시간. (초)
{
	return (float)nFrame / ( m_FrameSpeed * speed );
}

rmatrix* RSAnimationInfo::GetTransformMatrix(int iMesh,int iFrame)
{
//	_ASSERT((iFrame>=0)&&(iFrame<nFrame));
	if(iFrame<0 || iFrame>=nFrame) return NULL;
	if(!tm) return NULL;
	return &tm[iMesh*nFrame+iFrame];
}

int	RSAnimationInfo::isContainingVertexAnimation(int iMesh)
{
	if(!nVertAnim) return 0;
	return nVertAnim[iMesh];
}

rvector* RSAnimationInfo::GetAnimationVertices(int iMesh,int nF)
{
	if((nF>=nFrame) || (iMesh>=nMesh)) return NULL;
	return &vertanim[iMesh][nF*nVertAnim[iMesh]];
}

//#define WriteString(x) {if(x){fputc(strlen(x),file);fwrite(x,strlen(x),1,file);}else fputc(0,file);}
#define ReadString(x)	{int l=fgetc(file);if(l) {x=new char[l+1];x[l]=0;fread(x,l,1,file);}}
/*
bool RSAnimationInfo::Save(FILE *file)
{
	int i;
	WriteString(name);
	fwrite(&m_FrameSpeed,sizeof(float),1,file);
	fwrite(&speed,sizeof(float),1,file);
	fwrite(&nFrame,sizeof(int),1,file);
	fwrite(&nMesh,sizeof(int),1,file);
	fwrite(&m_AnimationMethod,sizeof(int),1,file);

	switch(m_AnimationMethod)
	{
	case RSANIMATIONMETHOD_TRANSFORM:
		{
			fwrite(tm,sizeof(rmatrix),nMesh*nFrame,file);
		}break;
	case RSANIMATIONMETHOD_VERTEX:
		{
			fwrite(nVertAnim,sizeof(int),nMesh,file);
			for(i=0;i<nMesh;i++)
				fwrite(vertanim[i],sizeof(rvector)*nVertAnim[i],nFrame,file);
		}break;
	}
	return true;
}
*/

bool RSAnimationInfo::Load(FILE *file)
{
	int i,j;
	
	ReadString(name);
	fread(&m_FrameSpeed,sizeof(float),1,file);
	fread(&speed,sizeof(float),1,file);
	fread(&nFrame,sizeof(int),1,file);
	fread(&nMesh,sizeof(int),1,file);
	fread(&m_AnimationMethod,sizeof(int),1,file);

	switch(m_AnimationMethod)
	{
	case RSANIMATIONMETHOD_TRANSFORM:
		{
			tm=new rmatrix[nMesh*nFrame];
			fread(tm,sizeof(rmatrix),nMesh*nFrame,file);
		}break;
	case RSANIMATIONMETHOD_VERTEX:
		{
			nVertAnim=new int[nMesh];
			vertanim=new rvector*[nMesh];
			for(i=0;i<nMesh;i++)
			{
				fread(nVertAnim+i,sizeof(int),1,file);
				if(nVertAnim[i])
				{
					vertanim[i]=new rvector[nVertAnim[i]*nFrame];
					fread(vertanim[i],sizeof(rvector)*nVertAnim[i],nFrame,file);
				}else
					vertanim[i]=NULL;
			}
		}break;
	}

	Visibility=new RSVisibilityControl[nMesh];
	for(i=0;i<nMesh;i++)
	{
		int nVisCount;
		fread(&nVisCount,sizeof(int),1,file);
		for(j=0;j<nVisCount;j++)
		{
			RSVisibilityKey *newkey=new RSVisibilityKey;
			fread(newkey,sizeof(RSVisibilityKey),1,file);
			Visibility[i].push_back(newkey);
		}
		Visibility[i].m_Current=Visibility[i].begin();
	}

	return true;
}

/*
float RSVisibilityControl::GetVisibility(float time)
{
	if(size()==0) return 1.0f;
	if(Current!=end() && time<(*Current)->time)
		Current=begin();
	while(Current!=end() && ((*Current)->time<=time) )
		Current++;
	if(Current==end())
	{
		Current--;
		return (*Current)->value;
	}

	if(Current==begin())
		return (*Current)->value;

	list <RSVisibilityKey*> :: iterator Next=Current;
	Current--;
	
	float t=((*Next)->time-time)/((*Next)->time-(*Current)->time);
	return (1.f-t)*(*Next)->value+t*(*Current)->value;
}
*/

void RSVisibilityControl::Get(float fTime,RSVisibilityKey *pRet)
{
	if(size()==0) { pRet->value=1.0f;return; }
	if(m_Current!=end() && fTime<(*m_Current)->time)
		m_Current=begin();
	while(m_Current!=end() && ((*m_Current)->time<=fTime) )
		m_Current++;
	if(m_Current==end())
	{
		m_Current--;
		pRet->value=min(max((*m_Current)->value,0),1);
		return;
	}

	if(m_Current==begin())
	{
		pRet->value=min(max((*m_Current)->value,0),1);
		return;
	}

	list <RSVisibilityKey*> :: iterator Next=m_Current;
	m_Current--;
	
	float t=((*Next)->time-fTime)/((*Next)->time-(*m_Current)->time);
	pRet->value=(1.f-t)*(*Next)->value+t*(*m_Current)->value;
	pRet->value=min(max(pRet->value,0),1);
	_ASSERT(pRet->value >= 0);
}