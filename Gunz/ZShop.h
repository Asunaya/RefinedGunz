#ifndef _ZSHOP_H
#define _ZSHOP_H

#include "ZPrerequisites.h"
#include <vector>
using namespace std;

class ZShop
{
private:
protected:
	int m_nPage;			// 샵에서 리스트 페이지
	bool m_bCreated;
	vector<unsigned long int>		m_ItemVector;

public:
	int m_ListFilter;

public:
	ZShop();
	virtual ~ZShop();
	bool Create();
	void Destroy();
	void Clear();
	void Serialize();

	bool CheckAddType(int type);

	int GetItemCount() { return (int)m_ItemVector.size(); }
	void SetItemsAll(unsigned long int* nItemList, int nItemCount);
	int GetPage() { return m_nPage; }
	unsigned long int GetItemID(int nIndex);
	static ZShop* GetInstance();
};

inline ZShop* ZGetShop() { return ZShop::GetInstance(); }

#endif