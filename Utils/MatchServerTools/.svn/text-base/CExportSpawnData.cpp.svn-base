#include "CExportSpawnData.h"
#include "MMatchStage.h"

CExportSpawnData::CExportSpawnData()
{

}

CExportSpawnData::~CExportSpawnData()
{

}

bool CExportSpawnData::Create(const char* szFolderName)
{
	strcpy(m_szMapFolderName, szFolderName);

	return true;
}

void CExportSpawnData::Export(const char* szOutputFileName)
{
	for (int i = 0; i < MMATCH_MAP_COUNT; i++)
	{
		char szFileName[256];
		sprintf(szFileName, "%s%s%s.RS.xml", m_szMapFolderName, g_MapDesc[i].szMapName, 
			g_MapDesc[i].szMapName);


	}
}

void CExportSpawnData::Destroy()
{

}
