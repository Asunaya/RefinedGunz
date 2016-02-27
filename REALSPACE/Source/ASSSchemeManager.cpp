#include "RSDebug.h"
#include "RSMaterialManager.h"
#include "ASSSchemeManager.h"
#include "RSMaterialList.h"
#include "RSD3D.h"
#include "rtexture.h"

ASSTile::ASSTile()
{
	nTextureHandle[0]=nTextureHandle[1]=nTextureHandle[2]=nTextureHandle[3]=0;
}

ASSTile::~ASSTile()
{
	for(int i=1;i<4;i++)
	{
		if(nTextureHandle[i])
		{
			RSDeleteTexture(nTextureHandle[i]);
			nTextureHandle[i]=NULL;
		}
	}
}

bool ASSSchemeManager::IsExtensionName(const char *name,int *nTileSet,int *nTileStyle,int *nTileAppearance)
{
	static char buf[256];
	static char seps[]   = "_";
	char *token;

	int n1,n2,n3;

	if(memcmp(name,"ext_",4)==0)
	{
		strcpy(buf,name+4);
		token = strtok(buf,seps);n1=atoi(token);if(!token) return false;
		token = strtok(NULL,seps);n2=atoi(token);if(!token) return false;
		token = strtok(NULL,seps);n3=atoi(token);if(!token) return false;
		token = strtok(NULL,seps);if(token) return false;
	}
	else
		return false;

	*nTileSet=n1;
	*nTileStyle=n2;
	*nTileAppearance=n3;

	return true;
}

bool ASSSchemeManager::Create(const char *RMLName)
{
	m_MaterialManager.Close();
	m_MaterialManager.SetUsePicMip(true);

	if(!m_MaterialManager.Open(RMLName))
		return false;
	
	int npTileSet=-1,npTileStyle=4,npTileAppearance=-1;
	int nTileSet,nTileStyle,nTileAppearance;

	int nCount=m_MaterialManager.GetCount(),i;
	for(i=0;i<nCount;i++)
	{
		RSMaterial *mat=m_MaterialManager.GetMaterial(i);
		if(memcmp(mat->Name,"base",4)==0)
		{
			ASSTile *newtile=new ASSTile;
			m_BaseTileList.Add(newtile);
			newtile->nTextureIndex=i;
			newtile->nTextureHandle[0]=(mat->TextureHandles)?mat->TextureHandles[0]:0;
			
			_ASSERT(newtile->nTextureHandle[0]);
			RSTexture *pTex=m_MaterialManager.GetMaterialList()->GetTexture(mat->Name);
			m_MaterialManager.GetMaterialList()->Lock(mat->Name);
			_ASSERT(pTex && pTex->Data);
			rtexture *tex=new rtexture;
			tex->Create(pTex->x,pTex->y,pTex->Data);
			for(int j=1;j<4;j++)
			{
				rtexture *tmptex=new rtexture;
				tmptex->New(pTex->x,pTex->y,RTEXTUREFORMAT_24);
				tmptex->Fill(0,0,j,tex);
				newtile->nTextureHandle[j]=RSCreateTexture(pTex->x,pTex->y,(char*)tmptex->GetData(),NULL,false,"rotated texture",m_MaterialManager.GetUsePicMip());
				delete tmptex;
			}
			delete tex;
			m_MaterialManager.GetMaterialList()->Unlock(mat->Name);
		}
		else
		if(IsExtensionName(mat->Name,&nTileSet,&nTileStyle,&nTileAppearance))
		{
			if((nTileStyle<0)||(nTileStyle>4))
			{
				rslog("Wrong Tile %s\n",mat->Name);
				Destroy();
				return false;
			}
			if(npTileSet!=nTileSet)	// »õ tile setÀÌ¸é.
			{
				if(npTileStyle!=4)
				{
					rslog("Some Tiles are missing before %s\n",mat->Name);
					Destroy();
					return false;
				}
				ASSTileSet *newtileset=new ASSTileSet;
				Add(newtileset);
			}

			/*
			if(!mat->TextureHandles) 
				return false;*/
			ASSTile *newtile=new ASSTile;
			newtile->nTextureHandle[0]=mat->TextureHandles?mat->TextureHandles[0]:0;

			_ASSERT(newtile->nTextureHandle[0]);
			RSTexture *pTex=m_MaterialManager.GetMaterialList()->GetTexture(mat->Name);
			m_MaterialManager.GetMaterialList()->Lock(mat->Name);
			_ASSERT(pTex && pTex->Data);
			rtexture *tex=new rtexture;
			tex->Create(pTex->x,pTex->y,pTex->Data);
			for(int j=1;j<4;j++)
			{
				rtexture *tmptex=new rtexture;
				tmptex->New(pTex->x,pTex->y,RTEXTUREFORMAT_24);
				tmptex->Fill(0,0,j,tex);
				newtile->nTextureHandle[j]=RSCreateTexture(pTex->x,pTex->y,(char*)tmptex->GetData(),NULL,false,"rotated texture",m_MaterialManager.GetUsePicMip());
				delete tmptex;
			}
			delete tex;
			m_MaterialManager.GetMaterialList()->Unlock(mat->Name);

			newtile->nTextureIndex=i;
			Get(GetCount()-1)->Get(nTileStyle)->Add(newtile);

			npTileSet=nTileSet;
			npTileStyle=nTileStyle;
			npTileAppearance=nTileAppearance;
		}
	}
	if(m_BaseTileList.GetCount()==0) 
	{
		Destroy();
		return false;
	}
	return true;
}

void ASSSchemeManager::Destroy()
{
	DeleteAll();
	m_MaterialManager.Close();
}

int ASSSchemeManager::GetTextureHandle(int index)
{
	return m_MaterialManager.Get(index);
}
