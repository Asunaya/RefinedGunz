#ifndef ZCHARACTERVIEWLIST_H
#define ZCHARACTERVIEWLIST_H

#include "ZPrerequisites.h"
#include "ZMeshViewList.h"
#include "MUID.h"
#include "ZGameInterface.h"

class ZCharacterView;
class MMatchObjCacheMap;
class MMatchObjCache;

class ZCharacterViewList : public ZMeshViewList{
protected:
	virtual bool OnShow(void);
	virtual void OnHide(void);
protected:
	MMatchCharItemParts m_nVisualWeaponParts;
public:
	ZCharacterViewList(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~ZCharacterViewList(void);

	//void Add(const char* szName);
	void Add(const MUID& uid, const char* szName, MMatchSex nSex, unsigned int nHair, unsigned int nFace, 
		unsigned int nLevel, unsigned long int* itemids, bool bFireWall = false);
	void Add(MMatchObjCache* pCache);

	void Remove(const MUID& uid);

	ZCharacterView* Get(const MUID& uid);

	void Assign(MMatchObjCacheMap* pObjCacheMap);

	void OnDraw(MDrawContext* pDC);

	void SetSelectCharacter(ZCharacterView* pSelectView);

	void ChangeMyCharacterInfo();
	void ChangeCharacterInfo();

	virtual bool OnCommand(MWidget* pWidget, const char* szMessage);

	MLabel*	GetLobbyCharNameViewer();
	MLabel*	GetStageCharNameViewer();

	ZCharacterView* GetLobbyCharViewer();
	ZCharacterView* GetStageCharViewer();
	ZCharacterView* GetLobbyCharEqViewer();
	ZCharacterView* GetLobbyCharShopViewer();

	void ChangeLobbyCharacterView(ZCharacterView* pSelectView);
	void ChangeStageCharacterView(ZCharacterView* pSelectView);

	void RefreshCharInfo();

//	void UpdateSelectView();
	ZCharacterView* FindSelectView(MUID uid);

public:

	MLabel* m_pLobbyCharNameViewer;
	MLabel* m_pStageCharNameViewer;

	ZCharacterView* m_pLobbyCharViewer;
	ZCharacterView* m_pStageCharViewer;
	ZCharacterView* m_pLobbyCharEqViewer;
	ZCharacterView* m_pLobbyCharShopViewer;


private:
//	ZCharacterView* m_SelectView;
//	ZCharacterView* m_MySelectView;
	MUID			m_MyUid;
	MUID			m_SelectViewUID;

public:
	void ChangeVisualWeaponParts(MMatchCharItemParts nVisualWeaponParts);
};

ZCharacterViewList* ZGetCharacterViewList(GunzState nState);

#endif