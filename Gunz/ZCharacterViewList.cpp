#include "stdafx.h"

#include "ZCharacterViewList.h"
#include "ZCharacterView.h"
#include "ZIDLResource.h"
#include "ZApplication.h"
#include "MMatchObjCache.h"
#include "ZMyInfo.h"
#include "ZGameClient.h"
#include "zpost.h"

ZCharacterViewList* ZGetCharacterViewList(GunzState nState)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	if (GUNZ_LOBBY == nState)
		return (ZCharacterViewList*)pResource->FindWidget("ChannelPlayerList");
	else if ((GUNZ_STAGE == nState) || (GUNZ_GAME == nState))
		return (ZCharacterViewList*)pResource->FindWidget("StagePlayerList");

	return NULL;
}

bool ZCharacterViewList::OnShow(void)
{
	return true;
}

void ZCharacterViewList::OnHide(void)
{
	RemoveAll();
}

ZCharacterViewList::ZCharacterViewList(const char* szName, MWidget* pParent, MListener* pListener)
: ZMeshViewList(szName, pParent, pListener)
{
	/*
	for(int i=0; i<10; i++){
		char temp[256];
		sprintf_safe(temp, "Test%d", i);
		Add(MUID(0,0), temp);
	}
	*/

	m_nVisualWeaponParts = MMCIP_PRIMARY;

//	m_SelectView = NULL;

	m_pLobbyCharViewer = NULL;
	m_pStageCharViewer = NULL;
	m_pLobbyCharEqViewer = NULL;
	m_pLobbyCharShopViewer = NULL;

	m_pLobbyCharNameViewer = NULL;
	m_pStageCharNameViewer = NULL;

//	m_MySelectView = NULL;

	m_SelectViewUID = MUID(0,0);
	m_MyUid	= MUID(0,0);
}

ZCharacterViewList::~ZCharacterViewList(void)
{
}

bool ZCharacterViewList::OnCommand(MWidget* pWidget, const char* szMessage)
{
	if(ZMeshViewList::OnCommand(pWidget,szMessage)==false) {

		if( strcmp(szMessage, MBTN_CLK_MSG)==0 ) {

			ZCharacterView* pSelectView = (ZCharacterView*)pWidget;
			SetSelectCharacter( pSelectView );

			return true;
		}
		else if(strcmp(szMessage, MBTN_RCLK_MSG)==0) {
			// 차후 메뉴로 수정~
			if((GetKeyState(VK_MENU)&0x8000)!=0) {

				ZCharacterView* pCharView = (ZCharacterView*) pWidget;

				if(pCharView) {
					char temp[1024];
					sprintf_safe(temp,"/kick %s", pCharView->m_szName);
					ZPostStageChat(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), temp);
				}
			}
		}

		return false;
	}
	return true;
}

void ZCharacterViewList::Add(const MUID& uid, const char* szName, MMatchSex nSex, 
		 unsigned int nHair, unsigned int nFace, unsigned int nLevel, unsigned long int* itemids, bool bFireWall)
{
	if (Get(uid) != NULL)		return;
	if (strlen(szName) <= 0)	return;

	ZCharacterView* pNew = new ZCharacterView(szName, this, this);
	pNew->SetDrawInfo(true);
	pNew->m_Info.UID = uid;
	pNew->m_Info.nLevel = nLevel;
	pNew->InitCharParts(nSex, nHair, nFace, itemids, m_nVisualWeaponParts);

	if (bFireWall)
	{
		pNew->m_Info.bFireWall = true;
	}

	RecalcBounds();

	/////////////////////////////////////////////////////////
	// 내 캐릭터가 추가되는 경우 상세정보 뷰어 바꾸기

	bool bMyChar = false;

	if(ZGetGameClient())
		if(ZGetGameClient()->GetPlayerUID()==uid) 
			bMyChar = true;
		
	if( bMyChar ) {		//	캐릭터 장비 초기화

//		m_MySelectView = pNew;
		m_MyUid = pNew->m_Info.UID;
		m_SelectViewUID = pNew->m_Info.UID;

		ChangeMyCharacterInfo();
		SetSelectCharacter(pNew);
	}

	/////////////////////////////////////////////////////////

}

void ZCharacterViewList::Add(MMatchObjCache* pCache)
{
	unsigned long int nItemids[MMCIP_END];
	memset(nItemids, 0, sizeof(nItemids));

	for (int i = 0; i < MMCIP_END; i++)
	{
		nItemids[i] = pCache->GetCostume()->nEquipedItemID[i];
	}

	Add(pCache->GetUID(), pCache->GetName(), pCache->GetCostume()->nSex,
		pCache->GetCostume()->nHair, pCache->GetCostume()->nFace, pCache->GetLevel(),
		nItemids, !pCache->CheckFlag(MTD_PlayerFlags_BridgePeer));

	/*
	ZCharacterView* pView = pWidget->Get(pCache->GetUID());
	if ( (pView) && (pCache->CheckFlag(MOBJCACHEFLAG_BRIDGEPEER)==false) )
	pView->m_bFireWall = true;
	*/

}

/*
void ZCharacterViewList::UpdateSelectView()
{
	int index = GetChildIndex(m_SelectView);

	if(index==-1) {

		SetSelectCharacter(m_MySelectView);
		m_SelectView = m_MySelectView;

	}
}
*/

ZCharacterView* ZCharacterViewList::FindSelectView(MUID uid)
{
	ZCharacterView* pCV = NULL;

	for(int i=0;i<GetChildCount();i++) {
		pCV = (ZCharacterView*)GetChild(i);
		if(pCV) {
			if(pCV->m_Info.UID==uid) {
				return pCV;
			}
		}
	}

	return NULL;
}

// 장비창 상점등에서 자신의 캐릭터가 변경된 경우 바뀐장비들을 갱신한다~
// 뷰 리스트는 갱신하고 장비창과 상점창갱신 + 뷰어는 선택된것이 자신일 경우만~

void ZCharacterViewList::ChangeCharacterInfo()
{
	ZMyInfo* pmi = ZGetMyInfo();

	if(pmi==NULL) return;

	ChangeMyCharacterInfo();//자신의 정보는 바꾸고~

	ZCharacterView* pCV = FindSelectView(m_SelectViewUID);
	ZCharacterView* pMyCV = FindSelectView(m_MyUid);

	if(pCV == NULL) {//없어졌다~, 내 캐릭으로 대체
		SetSelectCharacter(pMyCV);
	}
	else if(pCV==pMyCV) {
		SetSelectCharacter(pMyCV);
	}
	else {//다른사람은?
	}
}

void ZCharacterViewList::Remove(const MUID& uid)
{
	ZCharacterView* pCharacterView = Get(uid);

	if(pCharacterView) {
/*	
		if (pCharacterView->m_Info.UID == m_SelectViewUID) {
			ChangeMyCharacterInfo();
			m_SelectViewUID = MUID(0,0);
		}
*/
		delete pCharacterView;

		RecalcBounds();
	}
}

ZCharacterView* ZCharacterViewList::Get(const MUID& uid)
{
	for(int j=0; j<GetItemCount(); j++){
		ZCharacterView* pChild = (ZCharacterView*)GetItem(j);
		if(pChild->m_Info.UID==uid) return pChild;
	}
	return NULL;
}

void ZCharacterViewList::Assign(MMatchObjCacheMap* pObjCacheMap)
{
	m_SelectViewUID = MUID(0,0);

	RemoveAll();

	for (MMatchObjCacheMap::iterator itor = pObjCacheMap->begin(); itor != pObjCacheMap->end(); ++itor)
	{
		MMatchObjCache* pObj = (*itor).second;

		Add(pObj);
	}

	ZGetGameClient()->OutputMessage("CharacterViewList Assign", MZMOM_LOCALREPLY);
}

MLabel* ZCharacterViewList::GetLobbyCharNameViewer()
{
	if(m_pLobbyCharNameViewer==NULL) {
		m_pLobbyCharNameViewer = (MLabel*)ZGetGameInterface()->GetIDLResource()->FindWidget("Lobby_Charviewer_NameLable");
	}
	return m_pLobbyCharNameViewer;

}

MLabel* ZCharacterViewList::GetStageCharNameViewer()
{
	if(m_pStageCharNameViewer==NULL) {
		m_pStageCharNameViewer = (MLabel*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_Charviewer_NameLable");
	}
	return m_pStageCharNameViewer;
}

ZCharacterView* ZCharacterViewList::GetLobbyCharViewer()
{
	if(m_pLobbyCharViewer==NULL) {
		m_pLobbyCharViewer = (ZCharacterView*)ZGetGameInterface()->GetIDLResource()->FindWidget("Lobby_Charviewer1_Char");
	}
	return m_pLobbyCharViewer;
}

ZCharacterView* ZCharacterViewList::GetStageCharViewer()
{
	if(m_pStageCharViewer==NULL) {
		m_pStageCharViewer = (ZCharacterView*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_Charviewer_Char");
	}
	return m_pStageCharViewer;
}

ZCharacterView* ZCharacterViewList::GetLobbyCharEqViewer()
{
	if(m_pLobbyCharEqViewer==NULL) {
		m_pLobbyCharEqViewer = (ZCharacterView*)ZGetGameInterface()->GetIDLResource()->FindWidget("EquipmentInformation");
	}
	return m_pLobbyCharEqViewer;
}

ZCharacterView* ZCharacterViewList::GetLobbyCharShopViewer()
{
	if(m_pLobbyCharShopViewer==NULL) {
		m_pLobbyCharShopViewer = (ZCharacterView*)ZGetGameInterface()->GetIDLResource()->FindWidget("EquipmentInformationShop");
	}
	return m_pLobbyCharShopViewer;
}

// 캐릭터 장비 상태등이 바뀔경우 호출

void ZCharacterViewList::RefreshCharInfo()
{

}

// 자신의 캐릭터가 바뀌는 경우 : 뷰어 리스트와 장비창과 상점 갱신

void ZCharacterViewList::ChangeMyCharacterInfo()
{
	ZMyInfo* pmi = ZGetMyInfo();
	ZMyItemList* pil = ZGetMyInfo()->GetItemList();

	unsigned long int nWeaponID = GetVisualWeaponID(pil->GetEquipedItemID(MMCIP_MELEE),
		pil->GetEquipedItemID(MMCIP_PRIMARY), pil->GetEquipedItemID(MMCIP_SECONDARY),
		pil->GetEquipedItemID(MMCIP_CUSTOM1), pil->GetEquipedItemID(MMCIP_CUSTOM2));

	unsigned long int nItemids[MMCIP_END];

	if(pmi) {

		for (int i = 0; i < MMCIP_END; i++)	{
			nItemids[i] = pil->GetEquipedItemID(MMatchCharItemParts(i));
		}

		int sex = pmi->GetSex();
		int hair = pmi->GetHair();
		int face = pmi->GetFace();

		if(GetLobbyCharShopViewer()) {
			GetLobbyCharShopViewer()->InitCharParts(pmi->GetSex(), pmi->GetHair(), pmi->GetFace(), nItemids);
		}

		if(GetLobbyCharEqViewer()) {
			GetLobbyCharEqViewer()->InitCharParts(pmi->GetSex(), pmi->GetHair(), pmi->GetFace(), nItemids);
		}
		// 리스트에서 자신의 정보 갱신

		ZCharacterView* pMyCV = FindSelectView(m_MyUid);

		if(pMyCV) {
			pMyCV->InitCharParts(pmi->GetSex(), pmi->GetHair(), pmi->GetFace(), nItemids);
		}
	}
}

// 다른캐릭터를 마우스 선택하는 경우 등에 호출
// 로비와 스테이지의 캐릭터 뷰를 바꾼다..

void ZCharacterViewList::ChangeLobbyCharacterView(ZCharacterView* pSelectView)
{
	if(GetLobbyCharViewer()) {
		GetLobbyCharViewer()->InitCharParts(pSelectView);
		m_SelectViewUID = pSelectView->m_Info.UID;
	}
	if(GetLobbyCharNameViewer()) {
		strcpy_safe(GetLobbyCharNameViewer()->m_szName,pSelectView->m_szName);
	}
}

void ZCharacterViewList::ChangeStageCharacterView(ZCharacterView* pSelectView)
{
	if(GetStageCharViewer()) {
		GetStageCharViewer()->InitCharParts(pSelectView);
		m_SelectViewUID = pSelectView->m_Info.UID;
	}
	if(GetStageCharNameViewer()) {
		strcpy_safe(GetStageCharNameViewer()->m_szName,pSelectView->m_szName);
	}
}

void ZCharacterViewList::SetSelectCharacter(ZCharacterView* pSelectView)
{
	if(pSelectView==NULL) return;

	int nState = ZApplication::GetGameInterface()->GetState();

	m_SelectViewUID = pSelectView->m_Info.UID;

	if(nState==GUNZ_LOBBY)		ChangeLobbyCharacterView(pSelectView);
	else if(nState==GUNZ_STAGE) ChangeStageCharacterView(pSelectView);

}

void ZCharacterViewList::OnDraw(MDrawContext* pDC)
{
 	ZMeshViewList::OnDraw(pDC);

	// 스테이지에서 팀플인경우 타이틀바에 레드,블루팀 수치표시...
	// 사람이 많은경우 몇대몇인지 구분이 안됨~

	if (ZApplication::GetGameInterface()->GetState() == GUNZ_STAGE) {

//		ChangeCharacterInfo();

		if (ZGetGameClient()->GetMatchStageSetting()->IsTeamPlay()) {

			int red = 0;
			int blue = 0;

 			int nCount = GetItemCount();

			for(int i=0; i<nCount; i++) {

				ZCharacterView* pChild = (ZCharacterView*) GetItem(i);

				if( pChild ) {

					if(pChild->m_Info.nTeam == MMT_RED)	red++;
					else								blue++;
				}
			}

			// 타이틀바위에 그려주자~
/*
			char temp[10];

			MRECT tr = MRECT( 40, 4, 12, 12);
			pDC->SetColor(255, 0, 0);
			pDC->FillRectangle(tr);
			tr = MRECT( 60, 4, 12, 12);
			sprintf_safe(temp,":%d",red);
			pDC->SetColor(255, 255, 255);
			pDC->Text(tr,temp);

			tr = MRECT( 80, 3, 12, 12);
			pDC->SetColor(0, 0, 255);
			pDC->FillRectangle(tr);
			tr = MRECT( 100, 4, 12, 12);
			sprintf_safe(temp,":%d",blue);
			pDC->SetColor(255, 255, 255);
			pDC->Text(tr,temp);
//*/
			char cTemp[10];
			sprintf_safe( cTemp, "Red : %02d", red );
			MLabel* pWidget;
			pWidget = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_Team_Red_Count" );
			if(pWidget != 0)
			{
				pWidget->SetTextColor(0xFFFF0000);
				pWidget->SetText( cTemp );
				pWidget->Show( true );
				sprintf_safe( cTemp, "Blue : %02d", blue );
				pWidget = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_Team_Blue_Count" );
				pWidget->SetTextColor(0xFF0000FF);
				pWidget->SetText( cTemp ); 
				pWidget->Show( true );
			}
		}
		else
		{
			MWidget* pWidget;
			pWidget = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_Team_Red_Count" );
			if(pWidget != 0) pWidget->Show( false );
			pWidget = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_Team_Blue_Count" );
			if(pWidget != 0) pWidget->Show( false );
		}
	}
}


void ZCharacterViewList::ChangeVisualWeaponParts(MMatchCharItemParts nVisualWeaponParts)
{
	int nCount = GetItemCount();

	for(int i=0; i<nCount; i++)
	{
		MWidget* pChild = GetItem(i);
		if (!strcmp(pChild->GetClassName(), MINT_CHARACTERVIEW))
		{
			ZCharacterView* pCharacterView = (ZCharacterView*)pChild;
			pCharacterView->ChangeVisualWeaponParts(nVisualWeaponParts);
		}
	}


	m_nVisualWeaponParts = nVisualWeaponParts;
}