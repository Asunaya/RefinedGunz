#ifndef _CEXPORT_SPAWNDATA_H
#define _CEXPORT_SPAWNDATA_H


class CExportSpawnData
{
private:
protected:
	char		m_szMapFolderName[256];
public:
	CExportSpawnData();
	virtual ~CExportSpawnData();
	bool Create(const char* szFolderName);
	void Export(const char* szOutputFileName);
	void Destroy();
};








#endif