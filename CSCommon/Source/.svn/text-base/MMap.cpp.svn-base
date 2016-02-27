#include "MMap.h"
#include <string.h>
#include "RealSpace2.h"
#include "RBspObject.h"
#include "MZFileSystem.h"
#include "MErrorTable.h"

MMap::MMap(int nID, const char* szName)
{
	m_nID = nID;
	strcpy(m_szName, szName);

	m_pBspObj = NULL;
}

MMap::~MMap(void)
{
	Destroy();
	MObjectGridMap::Destroy();
}

int MMap::Create(const char* szFileName, MZFileSystem* pZFS)
{
	if(m_pBspObj!=NULL) delete m_pBspObj;
	m_pBspObj = new RealSpace2::RBspObject;
	RealSpace2::RSetFileSystem(pZFS);
	if(m_pBspObj->Open(szFileName, RealSpace2::RBspObject::ROF_EXCEPTBSP)==false){
	//if(m_pBspObj->Open(szFileName, pZFS, RealSpace2::RBspObject::ROF_ALL)==false){
		delete m_pBspObj;
		m_pBspObj = NULL;
		return MERR_MAP_CANNOT_OPEN_FILE;
	}

	RealSpace2::rboundingbox bb = m_pBspObj->GetRootNode()->bbTree;
	//float fWidth = bb.maxx - bb.minx;
	//float fHeight = bb.maxy - bb.miny;
	//MObjectGridMap::Create(bb.minx-fWidth/2.0f, bb.miny-fHeight/2.0f, bb.maxx+fWidth/2.0f, bb.maxy+fHeight/2.0f, 10, 10);
	MObjectGridMap::Create(bb.minx, bb.miny, bb.maxx, bb.maxy, 10, 10);

	//m_pBspObj->Pick()

	return MOK;
}

void MMap::Destroy(void)
{
	MObjectGridMap::Destroy();
	if(m_pBspObj!=NULL) delete m_pBspObj;
	m_pBspObj = NULL;
}

void MMap::RandomLocate(void)
{
#define RANDOMRANGE	0.9f

#define RANDOMPRECISION	10000
	float fPos = (float)(rand()%RANDOMPRECISION)/(float)RANDOMPRECISION;
}



MMapManager::MMapManager(void)
{
	/*
	// Temporary Data
	push_back(new MMap("Seoul"));
	push_back(new MMap("Jeju"));
	*/
}

MMapManager::~MMapManager(void)
{
	for(iterator i=begin(); i!=end(); i++){
		delete *i;
	}
	clear();
}

MMap* MMapManager::Get(const char* szName)
{
	for(iterator i=begin(); i!=end(); i++){
		if(strcmp((*i)->GetName(), szName)==0) return (*i);
	}
	return NULL;
}

int MMapManager::GetID(const char* szName)
{
	int nCount = 0;
	for(iterator i=begin(); i!=end(); i++){
		if(strcmp((*i)->GetName(), szName)==0) return nCount;
		nCount++;
	}
	return NULL;
}
