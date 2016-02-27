#include <stdio.h>
#include "RSMaterialManager.h"
//#include "RSGlobal_Dll.h"
#include "FileInfo.h"
#include "rsdebug.h"
#include "RSD3D.h"

#define RS_RGB_24(r,g,b)	((DWORD) (((BYTE) (b)|((WORD) (g) << 8))|(((DWORD) (BYTE) (r)) << 16)))

RSMaterialManager::RSMaterialManager()
{
	m_pRML=NULL;
	m_bUsePicMip = false;
}

RSMaterialManager::~RSMaterialManager()
{
	Close();
	AnimMaterialList.DeleteRecordAll();
	m_bUsePicMip = false;
}

int RSMaterialManager::GetIndex(const char *name)
{
	if(!name) return -1;
	RSMaterial *mat;
	for(int i=0;i<m_pRML->GetCount();i++)
	{
		mat=m_pRML->Get(i);
		if(strcmp(mat->Name,name)==0)
			return i;
	}
	return -1;
}

int RSMaterialManager::Get(const char *name)
{
	return Get(GetIndex(name));
}

int RSMaterialManager::Get(int index)
{
	_ASSERT(m_pRML);  // open 되어있는 RML이어야 한다.
	static int failcount=0;
	static RSMaterial *mat;
	int ret=0;
	static char maskname[256],texname[256];

	if((index<0)||(index==65535)) return 0;
	if(failcount>50) return 0;

	mat=m_pRML->Get(index);
	if(!mat) return 0;
	if(mat->TextureHandles)		// 이미 로드한 텍스쳐.
		return mat->TextureHandles[0];
	if(mat->nTexture)
	{
		if(!mat->TextureHandles)
			mat->TextureHandles=new int[mat->nTexture];
		ZeroMemory(mat->TextureHandles,sizeof(int)*mat->nTexture);
	}
	for(int i=0;i<mat->nTexture;i++)
	{
		if(!m_pRSTextureHandles[mat->TextureIndices[i]].TextureHandle)		// load 되지 않았다면,
		{
#ifdef _DEBUG
			m_pbref[mat->TextureIndices[i]]=true;
#endif
			if(!m_pRML->Lock(mat->TextureIndices[i])) return NULL;
			RSTexture *tex=m_pRML->GetTexture(mat->TextureIndices[i]);
			_ASSERT(tex);
			strcpy(maskname,tex->Name);strcat(maskname,"_mask");
// texture이름+ "_mask" 텍스쳐가 존재하면 그 텍스쳐의 mask로 간주한다.
			int iMask=m_pRML->GetTextureIndex(maskname);
			RSTexture *Mask=NULL;
			Mask=m_pRML->GetTexture(iMask);
			if(iMask!=-1) 
			{
#ifdef _DEBUG
			m_pbref[iMask]=true;
#endif
				if(!m_pRML->Lock(iMask) || (Mask->x!=tex->x)||(Mask->y!=tex->y) ) 
				{
					m_pRML->Unlock(iMask);
					return NULL;
					rslog("%s : %s texture size does not match\n",tex->Name,Mask->Name);
				}
				mat->ShadeMode=RSSHADEMODE_ALPHAMAP;
			}
#ifdef _DEBUG
			sprintf(texname,"MM:%s,%s",mat->Name,tex->Name);
#endif
			ret=mat->TextureHandles[i]=RSCreateTexture(tex->x,tex->y,tex->Data,
				Mask?Mask->Data:NULL,m_bForceNotMipmap | (Mask&&Mask->Data),texname,m_bUsePicMip );//mat->bColorkey?true:false,mat->Colorkey);
			if(iMask!=-1) m_pRML->Unlock(iMask);
			m_pRML->Unlock(mat->TextureIndices[i]);
			if(ret)
			{
//				log("Texture Created %d!!!\n",ret);
//				RSTextureInfo *t=new RSTextureInfo;
//				t->TextureHandle=ret;
//				Textures.Add(t);
				m_pRSTextureHandles[mat->TextureIndices[i]].TextureHandle=ret;
				m_pRSTextureHandles[mat->TextureIndices[i]].bAlpha=mat->ShadeMode==RSSHADEMODE_ALPHAMAP;
			}
			else
				failcount++;
		}
		else
		{
			mat->TextureHandles[i]=m_pRSTextureHandles[mat->TextureIndices[i]].TextureHandle;
//			mat->bAlpha=m_pRSTextureHandles[mat->TextureIndices[i]].bAlpha;
		}
	}
	if((mat->nTexture>1)&&(!mat->bLocked))
	{
		mat->bLocked=TRUE;
		mat->thistime=0;
		AnimMaterialList.Add(mat);
	}
	mat->TextureHandle=mat->nTexture?mat->TextureHandles[0]:0;
	return mat->TextureHandle;
}

bool RSMaterialManager::Open(FILE *stream,bool bForceNotMipmap)
{
	m_pRML=new RSMaterialList;
	if(!m_pRML->Open(stream))
	{
		delete m_pRML;
		m_pRML=NULL;
		return FALSE;
	}

	m_pRSTextureHandles=new RSTextureHandleInfo[m_pRML->GetTextureCount()];
	for(int k=0;k<m_pRML->GetTextureCount();k++)
	{
		m_pRSTextureHandles[k].TextureHandle=0;
		m_pRSTextureHandles[k].RefCount=0;
	}

#ifdef _DEBUG
	m_pbref=new bool[m_pRML->GetTextureCount()];
	for(k=0;k<m_pRML->GetTextureCount();k++)
		m_pbref[k]=false;
#endif

	m_bForceNotMipmap=bForceNotMipmap;
	return true;
}

bool RSMaterialManager::Open(const char *RMLName,bool bForceNotMipmap)
{
	FILE *file=fopen(RMLName,"rb");
	if(!file) return false;
	bool bReturn=Open(file,bForceNotMipmap);
	m_pRML->SetName(RMLName);
	fclose(file);
	return bReturn;
}

bool RSMaterialManager::Close()
{
	if(!m_pRML) return false;
	for(int j=0;j<m_pRML->GetTextureCount();j++)
	{
		if(m_pRSTextureHandles[j].TextureHandle)
			RSDeleteTexture(m_pRSTextureHandles[j].TextureHandle);
	}

//#define DUMP_NEVER_REF
#ifdef _DEBUG
#ifdef DUMP_NEVER_REF
	for(j=0;j<m_pRML->GetTextureCount();j++)
	{
		if(!m_pbref[j])
			rslog("never ref. texture : %s : %s \n",m_pRML->GetName(),m_pRML->GetTexture(j)->Name);
	}
#endif // of DUMP_NEVER_REF
	delete []m_pbref;m_pbref=NULL;
#endif

	delete []m_pRSTextureHandles;m_pRSTextureHandles=NULL;
	delete m_pRML;m_pRML=NULL;
	AnimMaterialList.DeleteRecordAll();
	return TRUE;
}

void RSMaterialManager::IncreaseRefCount(int index)
{
	if((index<0)||(index==65535)) return;
	_ASSERT(m_pRML);  // open 되어있는 RML이어야 한다.
	RSMaterial *mat=m_pRML->Get(index);
	for(int i=0;i<mat->nTexture;i++)
		m_pRSTextureHandles[mat->TextureIndices[i]].RefCount++;
}

void RSMaterialManager::DecreaseRefCount(int index)
{
	if((index<0)||(index==65535)) return;
	int rval=0;
	_ASSERT(m_pRML);  // open 되어있는 RML이어야 한다.
	RSMaterial *mat=m_pRML->Get(index);
	for(int i=0;i<mat->nTexture;i++)
	{
		rval=m_pRSTextureHandles[mat->TextureIndices[i]].RefCount--;
		_ASSERT(rval>=0); // 0이하가 되는경우는 없다.
		if(rval==0) RSDeleteTexture(
			m_pRSTextureHandles[mat->TextureIndices[i]].TextureHandle);
	}
}

void RSMaterialManager::MoveAnimation(DWORD time)
{
	for(int i=0;i<AnimMaterialList.GetCount();i++)
	{
		RSMaterial *mat=AnimMaterialList.Get(i);
		mat->thistime+=time;
		mat->TextureHandle=mat->TextureHandles[(mat->thistime/mat->AnimationSpeed)%mat->nTexture];
	}
}

RSMaterial* RSMaterialManager::GetMaterial(int index)
{
	_ASSERT(m_pRML);  // open 되어있는 RML이어야 한다.
	if(index==-1) return NULL;
	Get(index);
	return m_pRML->Get(index);
}

RSMaterial* RSMaterialManager::GetMaterial(const char *name)
{
	return GetMaterial(GetIndex(name));
}