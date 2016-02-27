// RSMaterialManager 99.10.19. dubble
// it is new implementation of RSTextureCache. and wrapper of RSMaterialList with Texture Management.

#ifndef __RSMATERIALMANAGER_H
#define __RSMATERIALMANAGER_H

#include "RSMaterialList.h"

struct RSTextureHandleInfo
{
	int TextureHandle;
	int RefCount;
	BOOL bAlpha;
};

class RSMaterialManager  
{

int MaxTexture;

public:
	RSMaterialManager();
	virtual ~RSMaterialManager();

	void IncreaseRefCount(int);
	void DecreaseRefCount(int);

	bool Open(const char *RMLName,bool bForceNotMipmap=false);
	bool Open(FILE *stream,bool bForceNotMipmap);
	bool Close();
	
	RSMaterialList *GetMaterialList() { return m_pRML; }
	FILETIME GetTimeStamp() {return TimeStamp;}

	int GetCount() { return m_pRML ? m_pRML->GetCount() : 0; }
	
	// get의 return은 texture handle임. Animation Texture인 경우는 첫번째.
	int Get(int index);
	int Get(const char *name);


	RSMaterial* GetMaterial(int index);
	RSMaterial* GetMaterial(const char *name);
	int GetIndex(const char *name);

	void MoveAnimation(DWORD time);

	void SetUsePicMip(bool b) { m_bUsePicMip = b; }
	bool GetUsePicMip(void)   { return m_bUsePicMip; }

private:
	RSMaterialList		*m_pRML;
	RSTextureHandleInfo	*m_pRSTextureHandles;
	FILETIME			TimeStamp;
	CMLinkedList <RSMaterial> AnimMaterialList;
	bool				m_bForceNotMipmap;
	bool				m_bUsePicMip;

#ifdef _DEBUG
	bool	*m_pbref;
#endif
};

#endif