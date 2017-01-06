#pragma once
#include <MXml.h>

struct GGSet
{
	UINT  WeaponSet[3];
};

class GunGame
{
private:
	bool ParseXML_ItemSet(MXmlElement& elem, GGSet& Node);
	std::vector<std::vector<GGSet>> m_Set;
public:
	GunGame();
	~GunGame();
	bool			ReadXML(const char* szFileName);
	static GunGame* GetInstance();	

	std::vector<GGSet> GetGGSet(int set){
		auto itor = m_Set.at(set);
		if (itor.size() > 0){
			return itor;
		}
		return {};
	}

	int GetSetCount()
	{
		return (int)m_Set.size();
	}

}; inline GunGame* MGetGunGame() { return GunGame::GetInstance(); }