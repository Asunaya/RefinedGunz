#ifndef _ZGAMEINTERFACE_H
#define _ZGAMEINTERFACE_H

#include "ZPrerequisites.h"
#include "ZInterface.h"
#include "ZCamera.h"
#include "ZChat.h"
#include "ZQuest.h"
#include "ZGameType.h"
#include "ZTips.h"
#include "ZScreenDebugger.h"
#include "ZCombatMenu.h"
#include "ZMyCharacter.h"
/*
#include "ZGame.h"
#include "ZCharacter.h"
#include "ZCombatInterface.h"
#include "ZObserver.h"
#include "ZLoading.h"
#include "ZGameInput.h"
#include "ZMyItemList.h"
#include "ZMonsterBookInterface.h"
#include "ZInitialLoading.h"
*/

// 여기에 #include 를 달기전에 꼭 그래야만하는지 다시한번 생각해보세요 +_+  - dubble


#define LOGINSTATE_FADEIN				0
#define LOGINSTATE_SHOWLOGINFRAME		1
#define LOGINSTATE_STANDBY				2
#define LOGINSTATE_LOGINCOMPLETE		3
#define LOGINSTATE_FADEOUT				4

class ZLocatorList;
class ZGameInput;
class ZMonsterBookInterface;

enum ZChangeWeaponType;

class MUserDataListItem : public MDefaultListItem{
	int m_nUserData;
public:
	MUserDataListItem(const char* szText, int nUserData)
		: MDefaultListItem(szText){
			m_nUserData=nUserData;
		}

	int GetUserData() { return m_nUserData; }
};

class ZGameInterface : public ZInterface {
public:
	GunzState			m_nInitialState;
	bool				m_bTeenVersion;
	bool				m_bViewUI;
	bool				m_bTeamPlay;
	int					m_nShopTabNum;
	int					m_nEquipTabNum;

	bool				m_bLoginTimeout;
	DWORD				m_dwLoginTimeout;


//	int					m_nSelectedCharacter;
	
protected:
	ZScreenEffectManager *m_pScreenEffectManager;
	ZEffectManager*		m_pEffectManager;

	GunzState			m_nPreviousState;

	ZCombatInterface*	m_pCombatInterface;
	ZGameInput*			m_pGameInput;
	ZLoading*			m_pLoadingInterface;
	ZPlayerMenu*		m_pPlayerMenu;

	ZGameClient*		g_pGameClient;
	ZGame*				m_pGame;
	ZCamera				m_Camera;
	ZChat				m_Chat;
	ZQuest				m_Quest;					///< 퀘스트 관련들
	ZGameTypeManager	m_GameTypeManager;			///< 게임타입 관련들
//	ZClan				m_Clan;
	ZMiniMap			*m_pMiniMap;
	ZTips				m_Tips;
	ZScreenDebugger		m_ScreenDebugger;			///< 화면에 나오는 디버그 화면
	ZCombatMenu			m_CombatMenu;				///< 게임중 메뉴

	ZMyCharacter*		m_pMyCharacter;

	ZMonsterBookInterface* m_pMonsterBookInterface;


	
	bool				m_bShowInterface;

	bool				m_bCursor;					///< 커서를 사용할 수 있는 상태
	LPDIRECT3DSURFACE9	m_pCursorSurface;

	DWORD				m_dwFrameMoveClock;

	ZIDLResource		m_IDLResource;

	GunzState			m_nState;			///< 현재 상태
	bool				m_bLogin;			///< Login 되었는가?

	bool				m_bLoading;
	bool				m_bWaitingArrangedGame;

	int					m_nSellQuestItemCount;

	MBitmap				*m_pThumbnailBitmap;///< 맵 썸네일

	ZMsgBox*				m_pMsgBox;
	ZMsgBox*				m_pConfirmMsgBox;
	ZInterfaceBackground*	m_pBackground;
	ZCharacterSelectView*	m_pCharacterSelectView;

	bool				m_bOnEndOfReplay;		// 리플레이 보구나면 플레이어의 Level Percent가 바뀌기 때문에 리플레이 시작 전에
	int					m_nLevelPercentCache;	// m_bOnEndOfReplay를 true로 셋한 다음 m_nLevelPercentCache에 현재 LevelPercent
												// 값을 저장해 놓구서 끝나면 다시 복원한다. 좀 안좋은 구조... 방법이 없음. -_-;

	unsigned long int	m_nDrawCount;

	bool			m_bReservedWeapon;
	ZChangeWeaponType m_ReservedWeapon;

	bool			m_bLeaveBattleReserved;
	bool			m_bLeaveStageReserved;
	DWORD			m_dwLeaveBattleTime;


	int				m_nLoginState;
	DWORD			m_dwLoginTimer;
	DWORD			m_dwRefreshTime;
	int				m_nLocServ;

	MBitmapR2*		m_pRoomListFrame;							// 게임방 리스트 프레임 이미지
	MBitmapR2*		m_pBottomFrame;								// 하단 정보창 프레임 이미지
	MBitmapR2*		m_pClanInfo;								// 클랜 정보 이미지
	MBitmapR2*		m_pLoginBG;									// 로그인 배경 이미지
	MBitmapR2*		m_pLoginPanel;								// 로그인 패널 이미지

	ZLocatorList*	m_pLocatorList;
	ZLocatorList*	m_pTLocatorList;

	DWORD			m_dwTimeCount;								// 게임 경과시간 카운트. 청소년 자율규제 적용안 쓰불...
	DWORD			m_dwHourCount;								// 게임 경과시간(hour) 카운트. 청소년 자율규제 적용안 쓰불...

protected:
	static bool		OnGlobalEvent(MEvent* pEvent);
	virtual bool	OnEvent(MEvent* pEvent, MListener* pListener);
	bool			OnDebugEvent(MEvent* pEvent, MListener* pListener);
	virtual bool	OnCommand(MWidget* pWidget, const char* szMessage);
	static bool		OnCommand(MCommand* pCommand);

	bool ResizeWidget(const char* szName, int w, int h);
	bool ResizeWidgetRecursive( MWidget* pWidget, int w, int h);
	void SetListenerWidget(const char* szName, MListener* pListener);

	void UpdateCursorEnable();

//	void LoadCustomBitmap();
	bool InitInterface(const char* szSkinName,ZLoadingProgress *pLoadingProgress = NULL);
	bool InitInterfaceListener();
	void FinalInterface();

	void LoadBitmaps(const char* szDir, ZLoadingProgress *pLoadingProgress = NULL);

	void LeaveBattle();

	void OnGreeterCreate(void);
	void OnGreeterDestroy(void);

	void OnLoginCreate(void);
	void OnLoginDestroy(void);

	void OnNetmarbleLoginCreate(void);
	void OnNetmarbleLoginDestroy(void);

	void OnLobbyCreate(void);
	void OnLobbyDestroy(void);

	void OnStageCreate(void);
	void OnStageDestroy(void);

	void OnCharSelectionCreate(void);
	void OnCharSelectionDestroy(void);

	void OnCharCreationCreate(void);
	void OnCharCreationDestroy(void);

	void OnShutdownState();

#ifdef _BIRDTEST
	void OnBirdTestCreate();
	void OnBirdTestDestroy();
	void OnBirdTestUpdate();
	void OnBirdTestDraw();
	void OnBirdTestCommand(MCommand* pCmd);
#endif

	void OnUpdateGameMessage(void);

	void HideAllWidgets();

	void OnResponseShopItemList(unsigned long int* nItemList, int nItemCount);
	void OnResponseCharacterItemList(MUID* puidEquipItem, MTD_ItemNode* pItemNodes, int nItemCount);

	void OnDrawStateGame(MDrawContext* pDC);
	void OnDrawStateLogin(MDrawContext* pDC);
	void OnDrawStateLobbyNStage(MDrawContext* pDC);
	void OnDrawStateCharSelection(MDrawContext* pDC);


#ifdef _QUEST_ITEM
	void OnResponseCharacterItemList_QuestItem( MTD_QuestItemNode* pQuestItemNode, int nQuestItemCount );
	void OnResponseBuyQuestItem( const int nResult, const int nBP );
	void OnResponseSellQuestItem( const int nResult, const int nBP );
	void SetSellQuestItemConfirmFrame( void);
#endif

	void OnResponseServerStatusInfoList( const int nListCount, void* pBlob );
	void OnResponseBlockCountryCodeIP( const char* pszBlockCountryCode, const char* pszRoutingURL );

	// locator관련.
	void RequestServerStatusListInfo();

/*
	GUNZ_NA = 0,
	GUNZ_GAME = 1,
	GUNZ_LOGIN = 2,
	GUNZ_NETMARBLELOGIN = 3,
	GUNZ_LOBBY = 4,
	GUNZ_STAGE = 5,
	GUNZ_GREETER = 6,
	GUNZ_CHARSELECTION = 7,
	GUNZ_CHARCREATION = 8,
	GUNZ_PREVIOUS = 10,
	GUNZ_SHUTDOWN = 11,
	GUNZ_BIRDTEST
*/
public:
	ZGameInterface(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	~ZGameInterface();

	bool OnCreate(ZLoadingProgress *pLoadingProgress);
	void OnDestroy();

	void OnInvalidate();
	void OnRestore();

	bool Update(float fElapsed);
	void OnDraw(MDrawContext *pDC);

	void SetCursorEnable(bool bEnable);
	void OnResetCursor();
	bool IsCursorEnable() { return m_bCursor; }

	bool SetState(GunzState nState);
	GunzState GetState(void){ return m_nState; }
	
	void UpdateBlueRedTeam(void);		// 동환이가 추가

	void ChangeToCharSelection(void);	///< 캐릭터 선택으로 이동

	bool ChangeInterfaceSkin(const char* szNewSkinName);

	/// 해당하는 이름을 아이템으로 가진 위젯의 Visible상태를 바꾼다.
	bool ShowWidget(const char* szName, bool bVisible, bool bModal=false);
	void SetTextWidget(const char* szName, const char* szText);
	void EnableWidget(const char* szName, bool bEnable);

	// 나중에 지울것..우선 당장 테스트를 위해
	void TestChangeParts(int mode);
	void TestChangePartsAll();
	void TestChangeWeapon(RVisualMesh* pVMesh = NULL);
	void TestToggleCharacter();

	void ChangeParts(int mode);
	void ChangeWeapon(ZChangeWeaponType nType);
	void Reload();

	void RespawnMyCharacter();	// 혼자테스트할때 클릭하면 되살아난다.

	void ReserveLeaveStage();	// 스테이지에서 나갈때 일정시간 흐른뒤 나간다
	void ReserveLeaveBattle();	// 대기방으로 나갈떄 일정시간 흐른뒤 나간다
	void FinishGame(void);

	void SaveScreenShot();

	void ShowMessage(const char* szText, MListener* pCustomListenter=NULL, int nMessageID=0);
	void ShowConfirmMessage(const char* szText, MListener* pCustomListenter=NULL);
	void ShowMessage(int nMessageID);
	void ShowErrorMessage(int nErrorID);
	
	void ShowInterface(bool bShowInterface);
	bool IsShowInterface() { return m_bShowInterface; }

	void SetTeenVersion(bool bt) { m_bTeenVersion = bt; }
	bool GetTeenVersion() { return m_bTeenVersion; }

	void ChangeEquipPartsToolTipAll();
	void ClearEquipPartsToolTipAll( const char* szName);

	void OnCharSelect(void);


	// GunzState에 따른 Create/Destroy 핸들러
	bool OnGameCreate(void);
	void OnGameDestroy(void);
	void OnGameUpdate(float fElapsed);


	// 로비 UI 설정
//	void SetupPlayerListButton(int index=-1);
//	void SetupPlayerListTab();
	void OnArrangedTeamGameUI(bool bFinding);
	void InitLadderUI(bool bLadderEnable);
	void InitClanLobbyUI(bool bClanBattleEnable);
	void InitChannelFrame(MCHANNEL_TYPE nChannelType);

//	bool InitLocatorList( MZFileSystem* pFileSystem, const char* pszLocatorList );

	// 스테이지 UI 설정
	void SetMapThumbnail(const char* szMapName);
	void ClearMapThumbnail();
	void SerializeStageInterface();

	void EnableLobbyInterface(bool bEnable);
	void EnableStageInterface(bool bEnable);
	void ShowPrivateStageJoinFrame(const char* szStageName);

	void SetRoomNoLight( int d );

	// 상점및 장비
	void Sell(void);
	void SellQuestItem( void);
	void Buy(void);
	void BuyCashItem(void);
	int CheckRestrictBringAccountItem();
	void BringAccountItem(void);
	bool Equip(void);
	bool Equip(MMatchCharItemParts parts, MUID& uidItem);
	void RequestQuickJoin();
	void GetBringAccountItem();
	void ShowEquipmentDialog(bool bShow=true);
	void ShowShopDialog(bool bShow=true);
	void SelectShopTab(int nTabIndex);
	void SelectEquipmentTab(int nTabIndex);
	void SelectEquipmentFrameList( const char* szName, bool bOpen);
	void EnableCharSelectionInterface(bool bEnable);
	void SetKindableItem( MMatchItemSlotType nSlotType);
#ifdef _QUEST_ITEM
	void OpenSellQuestItemConfirm( void);
	void SellQuestItemCountUp( void);
	void SellQuestItemCountDn( void);
#endif

	// 캐릭터 선택
	void ChangeSelectedChar( int nNum);


	// 리플레이
	void ShowReplayDialog( bool bShow);
	void ViewReplay( void);


	void ShowMenu(bool bEnable);
	void Show112Dialog(bool bShow);
	bool IsMenuVisible();

	bool OpenMiniMap();
	bool IsMiniMapEnable();

	void SetupItemDescription( MMatchItemDesc* pItemDesc, const char *szTextArea1, const char *szTextArea2, const char *szTextArea3, const char *szIcon, ZMyItemNode* pRentalNode);
	void SetupItemDescription( MQuestItemDesc* pItemDesc, const char *szTextArea1, const char *szTextArea2, const char *szTextArea3, const char *szIcon);

	// 클랜생성이나 래더팀게임초대를 받을수 있는 상태인가 ?
	bool IsReadyToPropose();

	// 리플레이
	void OnReplay();

	void OnRequestNewHashValue( const char* szNewRandomValue );
	void OnResponseNewHashValue( const char* szNewSerialKey );

	void OnDisconnectMsg( const DWORD dwMsgID );
	void OnAnnounceDeleteClan( const string& strAnnounce );

	// 퀘스트 아이템 아이콘 비트맵 얻기(쓰는덴 많은데 마땅히 둘데가 없어서... -_-;)
	MBitmap* GetQuestItemIcon( int nItemID, bool bSmallIcon);


	// 바깥에서 얻을만한 인터페이스들
	ZScreenEffectManager* GetScreenEffectManager() { return m_pScreenEffectManager; }
	ZEffectManager* GetEffectManager()			{ return m_pEffectManager; }
	ZGameClient* GetGameClient(void){ return g_pGameClient; }
	ZGame* GetGame(void){ return g_pGame; }
	ZCombatInterface* GetCombatInterface(void) { return m_pCombatInterface; }
	ZCharacterSelectView* GetCharacterSelectView() { return m_pCharacterSelectView; }
	ZCharacter*	GetMyCharacter()				{ return (ZCharacter*)m_pMyCharacter; }
	ZCamera* GetCamera()						{ return &m_Camera; }
	ZIDLResource* GetIDLResource(void)			{ return &m_IDLResource; }
	ZChat* GetChat()							{ return &m_Chat; }
	ZPlayerMenu* GetPlayerMenu()				{ return m_pPlayerMenu; }
	ZMiniMap*	GetMiniMap()					{ return m_pMiniMap; }
	ZQuest* GetQuest()							{ return &m_Quest; }
	ZGameTypeManager* GetGameTypeManager()		{ return &m_GameTypeManager; }
	ZTips* GetTips()							{ return &m_Tips; }
	ZScreenDebugger* GetScreenDebugger()		{ return &m_ScreenDebugger; }
	ZCombatMenu*	 GetCombatMenu()			{ return &m_CombatMenu; }
	ZMonsterBookInterface* GetMonsterBookInterface()	{ return m_pMonsterBookInterface; }
};


#define BEGIN_WIDGETLIST(_ITEM, _IDLRESPTR, _CLASS, _INSTANCE)								\
{																							\
	MWidgetList WidgetList;																	\
	(_IDLRESPTR)->FindWidgets(WidgetList, _ITEM);											\
	for (MWidgetList::iterator itor = WidgetList.begin(); itor != WidgetList.end(); ++itor) \
{																							\
	if ((*itor) != NULL)																	\
{																							\
	_CLASS _INSTANCE = ((_CLASS)(*itor));

#define END_WIDGETLIST()		}}}


#define DEFAULT_INTERFACE_SKIN "Default"


#define WM_CHANGE_GAMESTATE		(WM_USER + 25)
void ZChangeGameState(GunzState state);		/// 쓰레드에 안전하기 위해서는 만듦


//void ZLoadBitmap(const char* szDir, const char* szFilter, bool bAddDirToAliasName = false);

#endif