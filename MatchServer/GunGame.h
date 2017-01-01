#pragma once
#include <MXml.h>
#include <map>
#include <vector>
#include "MZFileSystem.h"
struct GGSet
{
	UINT  WeaponSet[3];
};

class GunGame
{
private:
	bool ParseXML_ItemSet(MXmlElement& elem, GGSet* Node);
	std::map<int,std::vector<GGSet*>> m_GGSet;
public:
	GunGame();
	~GunGame();
	bool			ReadXML(const char* szFileName, MZFileSystem* pFileSystem = nullptr);
	static GunGame* GetInstance();	

	std::vector<GGSet*> GetGGSet(int set){
		auto itor = m_GGSet.find(set);
		if (itor != m_GGSet.end()){
			return (*itor).second;
		}
		return {};
	}

	int GetSetCount()
	{
		return (int)m_GGSet.size();
	}

}; inline GunGame* MGetGunGame() { return GunGame::GetInstance(); }