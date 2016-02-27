#pragma once

#include <vector>

class ZWorld;
class ZWorldManager : public vector<ZWorld*>
{
	int m_nCurrent;
	set<ZWorld*>	m_Worlds;

public:
	ZWorldManager(void);
	virtual ~ZWorldManager(void);

	void Destroy();

	void AddWorld(const char* szMapName);

	void Clear();
	bool LoadAll(ZLoadingProgress *pLoading );

	int GetCount() { return (int)size(); }

	ZWorld	*GetWorld(int i);

	ZWorld	*SetCurrent(int i);
	ZWorld	*GetCurrent() { return GetWorld(m_nCurrent); }

	void OnInvalidate();
	void OnRestore();
};
