/***********************************************************************
  ZStageInterface.cpp
  
  용  도 : 스테이지 인터페이스를 관리하는 클래스. 코드 관리상의 편리를 위해
           분리했음(사실 아직 완전히 다 분리 못했음. -_-;).
  작성일 : 11, MAR, 2004
  작성자 : 임동환
************************************************************************/


#include "stdafx.h"							// Include stdafx.h
#include "ZStageInterface.h"				// Include ZStageInterface.h
#include "ZStageSetting.h"					// Include ZStageSetting.h
#include "ZGameInterface.h"
#include "ZPlayerListBox.h"
#include "ZCombatMenu.h"
#include "ZEquipmentListBox.h"
#include "ZMyItemList.h"
#include "ZItemSlotView.h"
#include "ZMessages.h"
#include "ZLanguageConf.h"


/* 해야할 것들...

 1. ZStageSetting 관련 루틴을 여기로 옮겨와야 하는디...  -_-;
 2. 버튼 UI쪽도 역시 여기로 옮겨와야 하는데 졸라 꼬여있다...  -_-;
*/


/***********************************************************************
  ZStageInterface : public
  
  desc : 생성자
************************************************************************/
ZStageInterface::ZStageInterface( void)
{
	m_bPrevQuest = false;
	m_bDrawStartMovieOfQuest = false;
	m_pTopBgImg = NULL;
	m_pStageFrameImg = NULL;
	m_pItemListFrameImg = NULL;
	m_nListFramePos = 0;
	m_nStateSacrificeItemBox = 0;
}


/***********************************************************************
  ~ZStageInterface : public
  
  desc : 소멸자
************************************************************************/
ZStageInterface::~ZStageInterface( void)
{
	if ( m_pTopBgImg != NULL)
	{
		delete m_pTopBgImg;
		m_pTopBgImg = NULL;
	}

	if ( m_pStageFrameImg != NULL)
	{
		delete m_pStageFrameImg;
		m_pStageFrameImg = NULL;
	}
}


/***********************************************************************
  Create : public
  
  desc : 초기화
************************************************************************/
void ZStageInterface::OnCreate( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	// 초기화 해주고
	m_bPrevQuest = false;
	m_bDrawStartMovieOfQuest = false;
	m_nStateSacrificeItemBox = 0;		// Hide
	m_nGameType = MMATCH_GAMETYPE_DEATHMATCH_SOLO;
	m_SacrificeItem[ SACRIFICEITEM_SLOT0].RemoveItem();
	m_SacrificeItem[ SACRIFICEITEM_SLOT1].RemoveItem();

	ReadSenarioNameXML();

	MPicture* pPicture = (MPicture*)pResource->FindWidget( "Stage_SacrificeItemImage0");
	if ( pPicture)
		pPicture->SetOpacity( 255);
	pPicture = (MPicture*)pResource->FindWidget( "Stage_SacrificeItemImage1");
	if ( pPicture)
		pPicture->SetOpacity( 255);

	pPicture = (MPicture*)pResource->FindWidget( "Stage_MainBGTop");
	if ( pPicture)
		pPicture->SetBitmap( MBitmapManager::Get( "main_bg_t.png"));
	pPicture = (MPicture*)pResource->FindWidget( "Stage_FrameBG");
	if ( pPicture)
	{
		m_pStageFrameImg = new MBitmapR2;
		((MBitmapR2*)m_pStageFrameImg)->Create( "stage_frame.png", RGetDevice(), "interface/loadable/stage_frame.png");

		if ( m_pStageFrameImg != NULL)
			pPicture->SetBitmap( m_pStageFrameImg->GetSourceBitmap());
	}
	pPicture = (MPicture*)pResource->FindWidget( "Stage_ItemListBG");
	if ( pPicture)
	{
		m_pItemListFrameImg = new MBitmapR2;
		((MBitmapR2*)m_pItemListFrameImg)->Create( "itemlistframe.tga", RGetDevice(), "interface/loadable/itemlistframe.tga");

		if ( m_pItemListFrameImg != NULL)
			pPicture->SetBitmap( m_pItemListFrameImg->GetSourceBitmap());
	}
	MWidget* pWidget = (MWidget*)pResource->FindWidget( "Stage_ItemListView");
	if ( pWidget)
	{
		MRECT rect;
		rect = pWidget->GetRect();
		rect.x = -rect.w;
		m_nListFramePos = rect.x;
		pWidget->SetBounds( rect);
	}
	MLabel* pLabel = (MLabel*)pResource->FindWidget( "Stage_SenarioName");
	if ( pLabel)
		pLabel->SetText( "");
	pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_SenarioNameImg");
	if ( pWidget)
		pWidget->Show( false);
	MListBox* pListBox = (MListBox*)pResource->FindWidget( "Stage_SacrificeItemListbox");
	if ( pListBox)
		pListBox->RemoveAll();
	MTextArea* pDesc = (MTextArea*)pResource->FindWidget( "Stage_ItemDesc");
	if ( pDesc)
	{
		pDesc->SetTextColor( MCOLOR(0xFF808080));
		pDesc->SetText( "아이템을 화면 중앙에 있는 두개의 제단에 끌어놓음으로써 게임 레벨을 조정할 수 있습니다.");
	}

	ZApplication::GetGameInterface()->ShowWidget( "Stage_Flame0", false);
	ZApplication::GetGameInterface()->ShowWidget( "Stage_Flame1", false);

	MComboBox* pCombo = (MComboBox*)pResource->FindWidget("StageType");			// 게임 방식을 채로 방 나갔다가 다른 방 들어가면 버그 생기는거 수정 
	if ( pCombo)
		pCombo->CloseComboBoxList();

	pCombo = (MComboBox*)pResource->FindWidget("MapSelection");					// 맵리스트 연 채로 방 나갔다가 다른 방 들어가면 방장이 아닌데도 맵 바꿔지는 버그 수정
	if ( pCombo)
		pCombo->CloseComboBoxList();


	// 채널 리스트 박스는 닫아버림
	pWidget = (MWidget*)pResource->FindWidget( "ChannelListFrame");
	if ( pWidget)
		pWidget->Show( false);


	// 화면 업데 한번 해주삼~
	UpdateSacrificeItem();
	SerializeSacrificeItemListBox();

	// QL 초기화
	OnResponseQL( 0);
}


/***********************************************************************
  OnDestroy : public
  
  desc : 서버나 혹은 시스템의 요청으로부터 스테이지 화면을 새로 갱신하는 함수
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::OnDestroy( void)
{
	ZApplication::GetGameInterface()->ShowWidget( "Stage", false);

	MPicture* pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_MainBGTop");
	if ( pPicture)
		pPicture->SetBitmap( MBitmapManager::Get( "main_bg_t.png"));
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_FrameBG");
	if ( pPicture)
		pPicture->SetBitmap( NULL);
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_ItemListBG");
	if ( pPicture)
		pPicture->SetBitmap( NULL);

	if ( m_pTopBgImg != NULL)
	{
		delete m_pTopBgImg;
		m_pTopBgImg = NULL;
	}
	if ( m_pStageFrameImg != NULL)
	{
		delete m_pStageFrameImg;
		m_pStageFrameImg = NULL;
	}
	if ( m_pItemListFrameImg != NULL)
	{
		delete m_pItemListFrameImg;
		m_pItemListFrameImg = NULL;
	}

	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_CharacterInfo");
	if ( pWidget)
		pWidget->Enable( true);

	m_SenarioNameDesc.clear();
}


/***********************************************************************
  OnStageInterfaceSettup : public
  
  desc : 서버나 혹은 시스템의 요청으로부터 스테이지 화면을 새로 갱신하는 함수
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::OnStageInterfaceSettup( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	ZStageSetting::InitStageSettingGameType();
/*
	// 맵 종류를 설정한다.
	MComboBox* pCB = (MComboBox*)pResource->FindWidget( "MapSelection");
	if ( pCB)
	{
		int nSelected = pCB->GetSelIndex();

		InitMaps( pCB);

		if ( nSelected >= pCB->GetCount())
			nSelected = pCB->GetCount() - 1;

		pCB->SetSelIndex( nSelected);
	}
*/

	// CharListView의 Add, Remove, Update는 ZGameClient::OnObjectCache 에서 관리한다.
	MSTAGE_CHAR_SETTING_NODE* pMyCharNode = NULL;
	bool bMyReady = false;		// Ready 상태인지 아닌지...
	for ( MStageCharSettingList::iterator itor = ZGetGameClient()->GetMatchStageSetting()->m_CharSettingList.begin();
		itor != ZGetGameClient()->GetMatchStageSetting()->m_CharSettingList.end();  ++itor) 
	{
		MSTAGE_CHAR_SETTING_NODE* pCharNode = (*itor);

		// 나 자신일 경우
		if ( pCharNode->uidChar == ZGetGameClient()->GetPlayerUID()) 
		{
			pMyCharNode = pCharNode;
			if (pMyCharNode->nState == MOSS_READY)
				bMyReady = true;
			else
				bMyReady = false;
		}

		ZPlayerListBox* pItem = (ZPlayerListBox*)pResource->FindWidget( "StagePlayerList_");
		if ( pItem)
		{
			bool bMaster = false;

			if ( ZGetGameClient()->GetMatchStageSetting()->GetMasterUID() == pCharNode->uidChar)
				bMaster = true;
			
			pItem->UpdatePlayer( pCharNode->uidChar,(MMatchObjectStageState)pCharNode->nState,bMaster,MMatchTeam(pCharNode->nTeam));
		}
	}

	// 스테이지의 버튼 상태(게임시작, 난입, 준비완료)를 설정한다.
	ChangeStageButtons( ZGetGameClient()->IsForcedEntry(), ZGetGameClient()->AmIStageMaster(), bMyReady);

	// 스테이지의...
	ChangeStageGameSetting( ZGetGameClient()->GetMatchStageSetting()->GetStageSetting());
	
	// 난입 멤버일 경우에...
	if ( !ZGetGameClient()->AmIStageMaster() && ( ZGetGameClient()->IsForcedEntry()))
	{
		if ( pMyCharNode != NULL)
			ChangeStageEnableReady( bMyReady);
	}

	// 만약 난입으로 들어왔는데 다른 사람 다 나가서 내가 방장이 되었다면 난입모드 해제
	if ( (ZGetGameClient()->AmIStageMaster() == true) && ( ZGetGameClient()->IsForcedEntry()))
	{
		if ( ZGetGameClient()->GetMatchStageSetting()->GetStageState() == STAGE_STATE_STANDBY)
		{
			ZGetGameClient()->ReleaseForcedEntry();

			// 인터페이스관련
			ZApplication::GetGameInterface()->EnableWidget( "StageSettingCaller", true);	// 방설정 버튼
			ZApplication::GetGameInterface()->EnableWidget( "MapSelection", true);			// 맵선택 콤보박스
			ZApplication::GetGameInterface()->EnableWidget( "StageType", true);				// 게임방식 콤보박스
			ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer", true);		// 최대인원 콤보박스
			ZApplication::GetGameInterface()->EnableWidget( "StageRoundCount", true);		// 경기횟수 콤보박스
		}
		else	// 마스터인데 다른사람들 게임중이면 인터페이스 Disable
		{
			// 인터페이스관련
			ZApplication::GetGameInterface()->EnableWidget( "StageSettingCaller", false);	// 방설정 버튼
			ZApplication::GetGameInterface()->EnableWidget( "MapSelection", false);			// 맵선택 콤보박스
			ZApplication::GetGameInterface()->EnableWidget( "StageType", false);			// 게임방식 콤보박스
			ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer", false);		// 최대인원 콤보박스
			ZApplication::GetGameInterface()->EnableWidget( "StageRoundCount", false);		// 경기횟수 콤보박스
		}
	}


	// 화면 상단의 맵 이미지 설정하기
	MPicture* pPicture = 0;
	MBitmap* pBitmap = 0;
	char szMapName[256];
 	pPicture = (MPicture*)pResource->FindWidget( "Stage_MainBGTop");
	if ( pPicture)
	{
		sprintf_safe( szMapName, "interface/loadable/%s", MGetMapImageName( ZGetGameClient()->GetMatchStageSetting()->GetMapName()));

		// 임시 하드코딩 우에엥~~~
		if ( m_nGameType == MMATCH_GAMETYPE_QUEST)
			strcpy_safe( szMapName, "interface/loadable/map_Mansion.bmp");

		if ( m_pTopBgImg != NULL)
		{
			delete m_pTopBgImg;
			m_pTopBgImg = NULL;
		}

		m_pTopBgImg = new MBitmapR2;
		((MBitmapR2*)m_pTopBgImg)->Create( "TopBgImg.png", RGetDevice(), szMapName);

		if ( m_pTopBgImg != NULL)
			pPicture->SetBitmap( m_pTopBgImg->GetSourceBitmap());
	}
	
	// 정보창에 게임방제 설정하기
	MLabel* pLabel = (MLabel*)pResource->FindWidget( "StageNameLabel");
	if ( pLabel != 0)
	{
		char szStr[ 256];
		sprintf_safe( szStr, "%s > %s > %03d:%s", ZGetGameClient()->GetServerName(), ZMsg( MSG_WORD_STAGE), ZGetGameClient()->GetStageNumber(), ZGetGameClient()->GetStageName());
		pLabel->SetText( szStr);
	}

	// 상하단 스트라이프의 색상 바꾸기
#define SDM_COLOR			MCOLOR(255,0,0)
#define TDM_COLOR			MCOLOR(0,255,0)
#define SGD_COLOR			MCOLOR(0,0,255)
#define TGD_COLOR			MCOLOR(255,255,0)
#define ASSASIN_COLOR		MCOLOR(255,0,255)
#define TRAINING_COLOR		MCOLOR(0,255,255)
#define QUEST_COLOR			MCOLOR(255,255,255)
#define SURVIVAL_COLOR		MCOLOR(255,255,255)

	MCOLOR color;
	switch ( ZGetGameClient()->GetMatchStageSetting()->GetGameType() )
	{	
		case MMATCH_GAMETYPE_ASSASSINATE:
			color = ASSASIN_COLOR;
			break;

		case MMATCH_GAMETYPE_DEATHMATCH_SOLO:
			color = SDM_COLOR;
			break;

		case MMATCH_GAMETYPE_DEATHMATCH_TEAM:
		case MMATCH_GAMETYPE_DEATHMATCH_TEAM2:
			color = TDM_COLOR;
			break;

		case MMATCH_GAMETYPE_GLADIATOR_SOLO:
			color = SGD_COLOR;
			break;

		case MMATCH_GAMETYPE_GLADIATOR_TEAM:
			color = TGD_COLOR;
			break;

		case MMATCH_GAMETYPE_TRAINING:
			color = TRAINING_COLOR;
			break;

#ifdef _QUEST
		case MMATCH_GAMETYPE_SURVIVAL:
			color = QUEST_COLOR;
			break;

		case MMATCH_GAMETYPE_QUEST:
			color = SURVIVAL_COLOR;
			break;
#endif
		case MMATCH_GAMETYPE_BERSERKER:
			color = SDM_COLOR;
			break;

		case MMATCH_GAMETYPE_DUEL:
			color = SDM_COLOR;
			break;
		default:
			_ASSERT(0);
			color = MCOLOR(255,255,255,255);
	}
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Stage_StripBottom");
	if(pPicture != NULL && !pPicture->IsAnim())
	{		
        pPicture->SetBitmapColor( color );
		if(!(pPicture->GetBitmapColor().GetARGB() == pPicture->GetReservedBitmapColor().GetARGB()))
			pPicture->SetAnimation( 2, 700.0f);
	}
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Stage_StripTop");
	if(pPicture != NULL && !pPicture->IsAnim())	
	{
		pPicture->SetBitmapColor( color );
		if(!(pPicture->GetBitmapColor().GetARGB() == pPicture->GetReservedBitmapColor().GetARGB()))
			pPicture->SetAnimation( 3, 700.0f);		
	}
}


/***********************************************************************
  ChangeStageGameSetting : public
  
  desc : 이것도 게임 관련 인터페이스를 수정하는거 같은데... 왜이렇게 많이 나눠놓은거지? -_-;
         주로 화면의 전체적인 UI를 설정한다.
  arg  : pSetting = 스테이지 설정 정보
  ret  : none
************************************************************************/
void ZStageInterface::ChangeStageGameSetting( MSTAGE_SETTING_NODE* pSetting)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	m_nGameType = pSetting->nGameType;

	// Set map name
	SetMapName( pSetting->szMapName);

	// Is team game?
	ZApplication::GetGameInterface()->m_bTeamPlay = ZGetGameTypeManager()->IsTeamGame( pSetting->nGameType);


	// 관전 허용 여부 확인
	MComboBox* pCombo = (MComboBox*)pResource->FindWidget( "StageObserver");
	MButton* pObserverBtn = (MButton*)pResource->FindWidget( "StageObserverBtn");
	MLabel* pObserverLabel = (MLabel*)pResource->FindWidget( "StageObserverLabel");
	if ( pCombo && pObserverBtn && pObserverLabel)
	{
		if ( pCombo->GetSelIndex() == 1)
		{
			pObserverBtn->SetCheck( false);
			pObserverBtn->Enable( false);
			pObserverLabel->Enable( false);
		}
		else
		{
			pObserverBtn->Enable( true);
			pObserverLabel->Enable( true);
		}
	}

	// 청팀, 홍팀 상태 설정
	ZApplication::GetGameInterface()->UpdateBlueRedTeam();

	// 게임 방식에 따라서 UI를 변경한다
	MAnimation* pAniMapImg = (MAnimation*)pResource->FindWidget( "Stage_MapNameBG");
	bool bQuestUI = false;
	if ( (pSetting->nGameType == MMATCH_GAMETYPE_DEATHMATCH_SOLO) ||			// 데쓰매치 개인전이거나...
		 (pSetting->nGameType == MMATCH_GAMETYPE_GLADIATOR_SOLO) ||				// 칼전 개인전이거나...
		 (pSetting->nGameType == MMATCH_GAMETYPE_BERSERKER) ||					// 버서커모드이거나...
		 (pSetting->nGameType == MMATCH_GAMETYPE_TRAINING) ||					// 트레이닝이거나...
		 (pSetting->nGameType == MMATCH_GAMETYPE_DUEL))							// 듀얼모드 이면...
	{
		// 맵 이름 배경 이미지 변환
		if ( pAniMapImg)
			pAniMapImg->SetCurrentFrame( 0);

		// 퀘스트 UI 감춤
		bQuestUI = false;
	}
	else if ( (pSetting->nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM) ||		// 데쓰매치 팀전이거나...
		(pSetting->nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM2) ||			// 무한데스매치 팀전이거나...
		 (pSetting->nGameType == MMATCH_GAMETYPE_GLADIATOR_TEAM) ||				// 칼전 팀전이거나...
		 (pSetting->nGameType == MMATCH_GAMETYPE_ASSASSINATE))					// 암살전 이면...
	{
		// 맵 이름 배경 이미지 변환
		if ( pAniMapImg)
			pAniMapImg->SetCurrentFrame( 1);

		// 퀘스트 UI 감춤
		bQuestUI = false;
	}
	else if ( pSetting->nGameType == MMATCH_GAMETYPE_SURVIVAL)					// 서바이벌 모드이면...
	{
		// 맵 이름 배경 이미지 변환
		if ( pAniMapImg)
			pAniMapImg->SetCurrentFrame( 0);

		// 퀘스트 UI 감춤
		bQuestUI = false;
	}
	else if ( pSetting->nGameType == MMATCH_GAMETYPE_QUEST)						// 퀘스트 모드이면...
	{
		// 맵 이름 배경 이미지 변환
		if ( pAniMapImg)
			pAniMapImg->SetCurrentFrame( 2);

		// 퀘스트 UI 보임
		bQuestUI = true;
	}

	// 퀘스트 UI 설정
	ZApplication::GetGameInterface()->ShowWidget( "Stage_SacrificeItemImage0", bQuestUI);
	ZApplication::GetGameInterface()->ShowWidget( "Stage_SacrificeItemImage1", bQuestUI);
	ZApplication::GetGameInterface()->ShowWidget( "Stage_Lights0", bQuestUI);
	ZApplication::GetGameInterface()->ShowWidget( "Stage_Lights1", bQuestUI);
	ZApplication::GetGameInterface()->ShowWidget( "Stage_QuestLevel", bQuestUI);
	ZApplication::GetGameInterface()->ShowWidget( "Stage_QuestLevelBG", bQuestUI);
	ZApplication::GetGameInterface()->ShowWidget( "Stage_SacrificeItemButton0", bQuestUI);
	ZApplication::GetGameInterface()->ShowWidget( "Stage_SacrificeItemButton1", bQuestUI);

	if ( m_bPrevQuest != bQuestUI)
	{
		m_SacrificeItem[ SACRIFICEITEM_SLOT0].RemoveItem();
		m_SacrificeItem[ SACRIFICEITEM_SLOT1].RemoveItem();

		UpdateSacrificeItem();

		if ( bQuestUI)
		{
//			ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer", false);
			ZPostRequestSacrificeSlotInfo( ZGetGameClient()->GetPlayerUID());
			ZPostRequestQL( ZGetGameClient()->GetPlayerUID());
			OpenSacrificeItemBox();
		}
		else
		{
			MLabel* pLabel = (MLabel*)pResource->FindWidget( "Stage_SenarioName");
			if ( pLabel)
				pLabel->SetText( "");
			ZApplication::GetGameInterface()->ShowWidget( "Stage_SenarioNameImg", false);
//			if (ZGetGameClient()->AmIStageMaster())
//				ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer", true);

			HideSacrificeItemBox();
		}

		m_bPrevQuest = !m_bPrevQuest;
	}

	if ( (pSetting->nGameType == MMATCH_GAMETYPE_SURVIVAL) || (pSetting->nGameType == MMATCH_GAMETYPE_QUEST))
		ZApplication::GetGameInterface()->EnableWidget( "StageSettingCaller", false);


	// 라운드 선택 콤보박스 보이기
//	bool bShowRound = true;
//	if ( ( pSetting->nGameType == MMATCH_GAMETYPE_SURVIVAL) || ( pSetting->nGameType == MMATCH_GAMETYPE_QUEST))
//		bShowRound = false;
	
//	ZApplication::GetGameInterface()->ShowWidget( "StageRoundCountLabelBG", bShowRound);
//	ZApplication::GetGameInterface()->ShowWidget( "StageRoundCountLabel", bShowRound);
//	ZApplication::GetGameInterface()->ShowWidget( "StageRoundCount", bShowRound);


	// 라운드 or Kill
	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "StageRoundCountLabel");
	if ( pWidget)
	{
		if ((pSetting->nGameType == MMATCH_GAMETYPE_DEATHMATCH_SOLO) ||
			(pSetting->nGameType == MMATCH_GAMETYPE_GLADIATOR_SOLO) ||
			(pSetting->nGameType == MMATCH_GAMETYPE_TRAINING) ||
			(pSetting->nGameType == MMATCH_GAMETYPE_BERSERKER) ||
			(pSetting->nGameType == MMATCH_GAMETYPE_DUEL))
			pWidget->SetText( ZMsg(MSG_WORD_KILL));

		else
			pWidget->SetText( ZMsg(MSG_WORD_ROUND));
	}


	// 콤포넌트 업데이트
	ZStageSetting::ShowStageSettingDialog( pSetting, false);


	// 게임중 메뉴 수정 - 머하는 부분인지 알수 없음...
#ifdef _QUEST
	if ( ZGetGameTypeManager()->IsQuestDerived( pSetting->nGameType))
		ZApplication::GetGameInterface()->GetCombatMenu()->EnableItem( ZCombatMenu::ZCMI_BATTLE_EXIT, false);
	else
		ZApplication::GetGameInterface()->GetCombatMenu()->EnableItem( ZCombatMenu::ZCMI_BATTLE_EXIT, true);
#endif
}


/***********************************************************************
  ChangeStageButtons : public
  
  desc : 스테이지 내의 버튼들(게임시작, 난입, 준비완료)의 상태를 설정한다.
         게임 설정과 관련된 위젯의 UI를 설정한다.
  arg  : bForcedEntry = 난입 여부(true or false)
         bMaster = 방장 여부(true or false)
		 bReady = 준비 완료 여부(true or false)
  ret  : none
************************************************************************/
void ZStageInterface::ChangeStageButtons( bool bForcedEntry, bool bMaster, bool bReady)
{
	if ( bForcedEntry)	// 난입 모드
	{
		ZApplication::GetGameInterface()->ShowWidget( "GameStart", false);
		ZApplication::GetGameInterface()->ShowWidget( "StageReady", false);

		ZApplication::GetGameInterface()->ShowWidget( "ForcedEntryToGame", true);
		ZApplication::GetGameInterface()->ShowWidget( "ForcedEntryToGame2", true);

		ChangeStageEnableReady( false);
	}
	else
	{
		ZApplication::GetGameInterface()->ShowWidget( "ForcedEntryToGame", false);
		ZApplication::GetGameInterface()->ShowWidget( "ForcedEntryToGame2", false);

		ZApplication::GetGameInterface()->ShowWidget( "GameStart", bMaster);
		ZApplication::GetGameInterface()->ShowWidget( "StageReady", !bMaster);

		if ( bMaster)
			ChangeStageEnableReady( false);
		else
			ChangeStageEnableReady( bReady);
	}
}


/***********************************************************************
  ChangeStageEnableReady : public
  
  desc : 스테이지의 인터페이스(버튼 enable등)를 켜거나 끄는 함수
  arg  : true(=interface enable) or false(=interface disable)
  ret  : none
************************************************************************/
void ZStageInterface::ChangeStageEnableReady( bool bReady)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	ZApplication::GetGameInterface()->EnableWidget( "GameStart", !bReady);

	ZApplication::GetGameInterface()->EnableWidget( "StageTeamBlue",  !bReady);
	ZApplication::GetGameInterface()->EnableWidget( "StageTeamBlue2", !bReady);
	ZApplication::GetGameInterface()->EnableWidget( "StageTeamRed",  !bReady);
	ZApplication::GetGameInterface()->EnableWidget( "StageTeamRed2", !bReady);
	ZApplication::GetGameInterface()->EnableWidget( "Lobby_StageExit", !bReady);

	if ( (m_nGameType == MMATCH_GAMETYPE_SURVIVAL) || (m_nGameType == MMATCH_GAMETYPE_QUEST))
	{
		ZApplication::GetGameInterface()->EnableWidget( "Stage_SacrificeItemListbox", !bReady);
		ZApplication::GetGameInterface()->EnableWidget( "Stage_PutSacrificeItem",     !bReady);
		ZApplication::GetGameInterface()->EnableWidget( "Stage_SacrificeItemButton0", !bReady);
		ZApplication::GetGameInterface()->EnableWidget( "Stage_SacrificeItemButton1", !bReady);
		if ( ZGetGameClient()->AmIStageMaster())
		{
			ZApplication::GetGameInterface()->EnableWidget( "MapSelection", !bReady);
			ZApplication::GetGameInterface()->EnableWidget( "StageType", !bReady);
		}
//		ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer", false);
		ZApplication::GetGameInterface()->EnableWidget( "StageSettingCaller", false);
	}
	else
	{
		if ( ZGetGameClient()->AmIStageMaster())
		{
			ZApplication::GetGameInterface()->EnableWidget( "MapSelection", !bReady);
			ZApplication::GetGameInterface()->EnableWidget( "StageType", !bReady);
			ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer", !bReady);
			ZApplication::GetGameInterface()->EnableWidget( "StageRoundCount", !bReady);
			ZApplication::GetGameInterface()->EnableWidget( "StageSettingCaller", !bReady);
		}
		else
		{
			ZApplication::GetGameInterface()->EnableWidget( "MapSelection", false);
			ZApplication::GetGameInterface()->EnableWidget( "StageType", false);
			ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer", false);
			ZApplication::GetGameInterface()->EnableWidget( "StageRoundCount", false);
			ZApplication::GetGameInterface()->EnableWidget( "StageSettingCaller", false);
		}
	}
    
	BEGIN_WIDGETLIST( "Stage_OptionFrame", pResource, MButton*, pButton);
	pButton->Enable( !bReady);
	END_WIDGETLIST();

	BEGIN_WIDGETLIST( "EquipmentCaller", pResource, MButton*, pButton);
	pButton->Enable( !bReady);
	END_WIDGETLIST();
}


/***********************************************************************
  SetMapName : public
  
  desc : 맵 선택 콤보박스에 맵 이름을 하나씩 등록시킨다.
  arg  : szMapName = 등록시킬 맵 이름
  ret  : none
************************************************************************/
void ZStageInterface::SetMapName( const char* szMapName)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	if ( szMapName == NULL)
		return;

	MComboBox* pMapCombo = (MComboBox*)pResource->FindWidget( "MapSelection");
	if ( pMapCombo)
	{
		// 일단 임시 하드코딩(우에엥~ ㅠ.ㅠ)
		if ( m_nGameType == MMATCH_GAMETYPE_QUEST)
			pMapCombo->SetText( "Mansion");
		else
			pMapCombo->SetText( szMapName);
	}
}




/*
	여기서부터 새로 추가된 내용...

	왠만한건 리스너나 다른 곳에서 자동으로 호출되도록 해놨으나 아직 테스트인 관계로
	완벽한건 아님...  -_-;
*/



/***********************************************************************
  OpenSacrificeItemBox : public
  
  desc : 희생 아이템 선택 창 열기
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::OpenSacrificeItemBox( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MButton* pButton = (MButton*)pResource->FindWidget( "Stage_SacrificeItemBoxOpen");
	if ( pButton)
		pButton->Show( false);
	pButton = (MButton*)pResource->FindWidget( "Stage_SacrificeItemBoxClose");
	if ( pButton)
		pButton->Show( true);

	m_nStateSacrificeItemBox = 2;
	GetSacrificeItemBoxPos();
}


/***********************************************************************
  CloseSacrificeItemBox : public
  
  desc : 희생 아이템 선택 창 닫기
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::CloseSacrificeItemBox( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MButton* pButton = (MButton*)pResource->FindWidget( "Stage_SacrificeItemBoxClose");
	if ( pButton)
		pButton->Show( false);
	pButton = (MButton*)pResource->FindWidget( "Stage_SacrificeItemBoxOpen");
	if ( pButton)
		pButton->Show( true);

	MWidget* pWidget = pResource->FindWidget( "Stage_CharacterInfo");
	if ( pWidget)
		pWidget->Enable( true);

	m_nStateSacrificeItemBox = 1;
	GetSacrificeItemBoxPos();
}


/***********************************************************************
  HideSacrificeItemBox : public
  
  desc : 희생 아이템 선택 창 감추기
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::HideSacrificeItemBox( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MButton* pButton = (MButton*)pResource->FindWidget( "Stage_SacrificeItemBoxClose");
	if ( pButton)
		pButton->Show( false);
	pButton = (MButton*)pResource->FindWidget( "Stage_SacrificeItemBoxOpen");
	if ( pButton)
		pButton->Show( true);

	MWidget* pWidget = pResource->FindWidget( "Stage_CharacterInfo");
	if ( pWidget)
		pWidget->Enable( true);

	m_nStateSacrificeItemBox = 0;
	GetSacrificeItemBoxPos();
}


/***********************************************************************
  HideSacrificeItemBox : public
  
  desc : 희생 아이템 선택 창 위치 구하기
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::GetSacrificeItemBoxPos( void)
{
//#ifdef _DEBUG
//	m_nListFramePos = 0;
//	return;
//#endif

	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_ItemListView");
	if ( pWidget)
	{
		MRECT rect;

		switch ( m_nStateSacrificeItemBox)
		{
			case 0 :		// Hide
				rect = pWidget->GetRect();
				m_nListFramePos = -rect.w;
				break;

			case 1 :		// Close
				rect = pWidget->GetRect();
				m_nListFramePos = -rect.w + ( rect.w * 0.14);
				break;

			case 2 :		// Open
				m_nListFramePos = 0;
				break;
		}
	}
}


/***********************************************************************
  OnSacrificeItem0 : public
  
  desc : 희생 아이템0을 눌렀을 때
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::OnSacrificeItem0( void)
{
}


/***********************************************************************
  OnSacrificeItem1 : public
  
  desc : 희생 아이템1를 눌렀을 때
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::OnSacrificeItem1( void)
{
}


/***********************************************************************
  UpdateSacrificeItem : protected
  
  desc : 변경된 희생 아이템 이미지, 정보등을 업데이트 함.
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::UpdateSacrificeItem( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	// 스테이지 영역에 있는 희생 아이템 이미지 수정
	for ( int i = SACRIFICEITEM_SLOT0;  i <= SACRIFICEITEM_SLOT1;  i++)
	{
		char szWidgetNameItem[ 128];
		sprintf_safe( szWidgetNameItem, "Stage_SacrificeItemImage%d", i);
		MPicture* pPicture = (MPicture*)pResource->FindWidget( szWidgetNameItem);
		if ( pPicture)
		{
			if ( m_SacrificeItem[ i].IsExist())
			{
				pPicture->SetBitmap( m_SacrificeItem[ i].GetIconBitmap());
				char szMsg[ 128];
				MMatchObjCache* pObjCache = ZGetGameClient()->FindObjCache( m_SacrificeItem[ i].GetUID());
				if ( pObjCache)
					sprintf_safe( szMsg, "%s (%s)", m_SacrificeItem[ i].GetName(), pObjCache->GetName());
				else
					strcpy_safe( szMsg, m_SacrificeItem[ i].GetName());
				pPicture->AttachToolTip( szMsg);
			}
			else
			{
				pPicture->SetBitmap( NULL);
				pPicture->DetachToolTip();
			}
		}
	}
}


/***********************************************************************
  SerializeSacrificeItemListBox : public
  
  desc : 희생 아이템 리스트 박스에 자료를 받는다.
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::SerializeSacrificeItemListBox( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MListBox* pListBox = (MListBox*)pResource->FindWidget( "Stage_SacrificeItemListbox");
	if ( !pListBox)
		return;

	int nStartIndex  = pListBox->GetStartItem();
	int nSelectIndex = pListBox->GetSelIndex();
	pListBox->RemoveAll();

	// 리스트에 추가
	for ( MQUESTITEMNODEMAP::iterator questitem_itor = ZGetMyInfo()->GetItemList()->GetQuestItemMap().begin();
		  questitem_itor != ZGetMyInfo()->GetItemList()->GetQuestItemMap().end();
		  questitem_itor++)
	{
		ZMyQuestItemNode* pItemNode = (*questitem_itor).second;
		MQuestItemDesc* pItemDesc = GetQuestItemDescMgr().FindQItemDesc( pItemNode->GetItemID());
		if ( pItemDesc)
		{
			int nCount = pItemNode->m_nCount;
			if ( m_SacrificeItem[ SACRIFICEITEM_SLOT0].IsExist() &&
				 (m_SacrificeItem[ SACRIFICEITEM_SLOT0].GetUID() == ZGetGameClient()->GetPlayerUID()) &&
				 (pItemDesc->m_nItemID == m_SacrificeItem[ SACRIFICEITEM_SLOT0].GetItemID()))
				nCount--;
			if ( m_SacrificeItem[ SACRIFICEITEM_SLOT1].IsExist() &&
				 (m_SacrificeItem[ SACRIFICEITEM_SLOT1].GetUID() == ZGetGameClient()->GetPlayerUID()) &&
				 (pItemDesc->m_nItemID == m_SacrificeItem[ SACRIFICEITEM_SLOT1].GetItemID()))
				nCount--;

			if ( pItemDesc->m_bSecrifice && (nCount > 0))		// 희생 아이템만 추가
			{
				pListBox->Add( new SacrificeItemListBoxItem( pItemDesc->m_nItemID,
															 ZApplication::GetGameInterface()->GetQuestItemIcon( pItemDesc->m_nItemID, true),
															 pItemDesc->m_szQuestItemName,
															 nCount,
															 pItemDesc->m_szDesc));
			}
		}
	}

	MWidget* pWidget = pResource->FindWidget( "Stage_NoItemLabel");
	if ( pWidget)
	{
		if ( pListBox->GetCount() > 0)
			pWidget->Show( false);
		else
			pWidget->Show( true);
	}

	pListBox->SetStartItem( nStartIndex);
	pListBox->SetSelIndex( min( (pListBox->GetCount() - 1), nSelectIndex));
}


/***********************************************************************
  OnDropSacrificeItem : public
  
  desc : 희생 아이템 놓기
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::OnDropSacrificeItem( int nSlotNum)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MListBox* pListBox = (MListBox*)pResource->FindWidget( "Stage_SacrificeItemListbox");
	if ( !pListBox || (pListBox->GetSelIndex() < 0))
		return;

	SacrificeItemListBoxItem* pItemDesc = (SacrificeItemListBoxItem*)pListBox->Get( pListBox->GetSelIndex());
	if ( pItemDesc)
	{
		MTextArea* pDesc = (MTextArea*)pResource->FindWidget( "Stage_ItemDesc");

		// 슬롯이 비어있으면 무조건 올림
		if ( ! m_SacrificeItem[ nSlotNum].IsExist())
		{
			ZPostRequestDropSacrificeItem( ZGetGameClient()->GetPlayerUID(), nSlotNum, pItemDesc->GetItemID());

			if ( pDesc)
				pDesc->Clear();
		}

		// 슬롯이 비어있지 않으면...
		else
		{
			if ( (m_SacrificeItem[ nSlotNum].GetUID()    != ZGetGameClient()->GetPlayerUID()) ||
				 (m_SacrificeItem[ nSlotNum].GetItemID() != pItemDesc->GetItemID()))
				ZPostRequestDropSacrificeItem( ZGetGameClient()->GetPlayerUID(), nSlotNum, pItemDesc->GetItemID());

			if ( pDesc)
				pDesc->Clear();
		}
	}
}


/***********************************************************************
  OnRemoveSacrificeItem : public
  
  desc : 희생 아이템 빼기
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::OnRemoveSacrificeItem( int nSlotNum)
{
	if ( !m_SacrificeItem[ nSlotNum].IsExist())
		return;

	ZPostRequestCallbackSacrificeItem( ZGetGameClient()->GetPlayerUID(),
									   nSlotNum,
									   m_SacrificeItem[ nSlotNum].GetItemID());

	MTextArea* pDesc = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_ItemDesc");
	if ( pDesc)
		pDesc->Clear();
}


/***********************************************************************
  MSacrificeItemListBoxListener
  
  desc : 희생 아이템 리스트 박스 리스너
************************************************************************/
class MSacrificeItemListBoxListener : public MListener
{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		// On select
		if ( MWidget::IsMsg( szMessage, MLB_ITEM_SEL) == true)
		{
			MListBox* pListBox = (MListBox*)pWidget;

			// 아이템 설명 업데이트
			SacrificeItemListBoxItem* pItemDesc = (SacrificeItemListBoxItem*)pListBox->GetSelItem();
			MTextArea* pDesc = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_ItemDesc");
			if ( pItemDesc && pDesc)
			{
				char szCount[ 128];
				sprintf_safe( szCount, "%s : %d", ZMsg( MSG_WORD_QUANTITY), pItemDesc->GetItemCount());
				pDesc->SetTextColor( MCOLOR( 0xFFD0D0D0));
				pDesc->SetText( szCount);
				pDesc->AddText( "\n");
				pDesc->AddText( pItemDesc->GetItemDesc(), 0xFF808080);
			}

			return true;
		}


		// On double click
		else if ( MWidget::IsMsg( szMessage, MLB_ITEM_DBLCLK) == true)
		{
			// Put item
			if ( !ZApplication::GetStageInterface()->m_SacrificeItem[ 0].IsExist())
				ZApplication::GetStageInterface()->OnDropSacrificeItem( 0);
			else if ( !ZApplication::GetStageInterface()->m_SacrificeItem[ 1].IsExist())
				ZApplication::GetStageInterface()->OnDropSacrificeItem( 1);
		
			return true;
		}

		return false;
	}
};

MSacrificeItemListBoxListener g_SacrificeItemListBoxListener;

MListener* ZGetSacrificeItemListBoxListener( void)
{
	return &g_SacrificeItemListBoxListener;
}


/***********************************************************************
  OnDropCallbackRemoveSacrificeItem
  
  desc : 희생 아이템 제거
************************************************************************/
void OnDropCallbackRemoveSacrificeItem( void* pSelf, MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
	if ( (pSender == NULL) || (strcmp(pSender->GetClassName(), MINT_ITEMSLOTVIEW)))
		return;

	ZItemSlotView* pItemSlotView = (ZItemSlotView*)pSender;
	ZApplication::GetStageInterface()->OnRemoveSacrificeItem( (strcmp( pItemSlotView->m_szItemSlotPlace, "SACRIFICE0") == 0) ? 0 : 1);
}


/***********************************************************************
  StartMovieOfQuest : public
  
  desc : 퀘스트 모드로 시작할때 아이템 합쳐지는 무비를 시작함
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::StartMovieOfQuest( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	m_dwClockOfStartMovie = timeGetTime();

	// 화염 애니메이션 시작
	MAnimation* pAnimation = (MAnimation*)pResource->FindWidget( "Stage_Flame0");
	if ( pAnimation && m_SacrificeItem[ SACRIFICEITEM_SLOT0].IsExist())
	{
		pAnimation->SetCurrentFrame( 0);
		pAnimation->Show( true);
		pAnimation->SetRunAnimation( true);
	}
	pAnimation = (MAnimation*)pResource->FindWidget( "Stage_Flame1");
	if ( pAnimation && m_SacrificeItem[ SACRIFICEITEM_SLOT1].IsExist())
	{
		pAnimation->SetCurrentFrame( 0);
		pAnimation->Show( true);
		pAnimation->SetRunAnimation( true);
	}

	m_bDrawStartMovieOfQuest = true;
}


/***********************************************************************
  OnDrawStartMovieOfQuest : public
  
  desc : 퀘스트 모드로 시작할때 아이템 합쳐지는 무비를 그림
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::OnDrawStartMovieOfQuest( void)
{
	if ( !m_bDrawStartMovieOfQuest)
		return ;

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	// 경과 시간을 구한다.
	DWORD dwClock = timeGetTime() - m_dwClockOfStartMovie;

	// 희생 아이템 페이드 아웃
	int nOpacity = 255 - dwClock * 0.12f;
	if ( nOpacity < 0)
		nOpacity = 0;

	MPicture* pPicture = (MPicture*)pResource->FindWidget( "Stage_SacrificeItemImage0");
	if ( pPicture && m_SacrificeItem[ SACRIFICEITEM_SLOT0].IsExist())
		pPicture->SetOpacity( nOpacity);

	pPicture = (MPicture*)pResource->FindWidget( "Stage_SacrificeItemImage1");
	if ( pPicture && m_SacrificeItem[ SACRIFICEITEM_SLOT1].IsExist())
		pPicture->SetOpacity( nOpacity);

	// 종료 시간일 경우에...
	if ( dwClock > 3200)
	{
		m_bDrawStartMovieOfQuest = false;

		ZMyQuestItemMap::iterator itMyQItem;

		// 여기서 슬롯에 자신의 아이템이 올려져 있으면 해당 아이템 카운트 감소.
		if( ZGetGameClient()->GetUID() == m_SacrificeItem[ SACRIFICEITEM_SLOT0].GetUID() )
		{
			itMyQItem = ZGetMyInfo()->GetItemList()->GetQuestItemMap().find( m_SacrificeItem[ SACRIFICEITEM_SLOT0].GetItemID() );
			itMyQItem->second->Decrease();
		}
		if( ZGetGameClient()->GetUID() == m_SacrificeItem[ SACRIFICEITEM_SLOT1].GetUID() )
		{
			itMyQItem = ZGetMyInfo()->GetItemList()->GetQuestItemMap().find( m_SacrificeItem[ SACRIFICEITEM_SLOT1].GetItemID() );
			itMyQItem->second->Decrease();
		}		
		
		m_SacrificeItem[ SACRIFICEITEM_SLOT0].RemoveItem();
		m_SacrificeItem[ SACRIFICEITEM_SLOT1].RemoveItem();

		ZApplication::GetGameInterface()->SetState( GUNZ_GAME);
	}
}


/***********************************************************************
  IsShowStartMovieOfQuest : public
  
  desc : 퀘스트 모드로 시작할때 아이템 합쳐지는 무비를 보여줄지 여부를 결정.
  arg  : none
  ret  : true(=Quest mode start movie) or false(=none)
************************************************************************/
bool ZStageInterface::IsShowStartMovieOfQuest( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	if ( m_nGameType == MMATCH_GAMETYPE_QUEST)
	{
		if ( m_SacrificeItem[ SACRIFICEITEM_SLOT0].IsExist() || m_SacrificeItem[ SACRIFICEITEM_SLOT1].IsExist())
			return true;
	}

	return false;
}


/***********************************************************************
  OnResponseDropSacrificeItemOnSlot : public
  
  desc : 희생 아이템이 올라갔을때
  arg  : none
  ret  : none
************************************************************************/
bool ZStageInterface::OnResponseDropSacrificeItemOnSlot( const int nResult, const MUID& uidRequester, const int nSlotIndex, const int nItemID )
{
#ifdef _QUEST_ITEM
	if( MOK == nResult)
	{
		MQuestItemDesc* pItemDesc = GetQuestItemDescMgr().FindQItemDesc( nItemID);
		MBitmap* pIconBitmap = ZApplication::GetGameInterface()->GetQuestItemIcon( nItemID, false);

		m_SacrificeItem[ nSlotIndex].SetSacrificeItemSlot( uidRequester, nItemID, pIconBitmap, pItemDesc->m_szQuestItemName, pItemDesc->m_nLevel);
		SerializeSacrificeItemListBox();

		UpdateSacrificeItem();
	}
	else if( ITEM_TYPE_NOT_SACRIFICE == nResult)
	{
		// 희생 아이템이 아님.
		return false;
	}
	else if( NEED_MORE_QUEST_ITEM == nResult )
	{
		// 현제 가지고 있는 수량을 초과해서 올려 놓으려고 했을경우.
	}
	else if( MOK != nResult )
	{
		// 실패...
		return false;
	}
	else
	{
		// 정의되지 않은 error...
		ASSERT( 0 );
	}

#endif

	return true;
}


/***********************************************************************
  OnResponseCallbackSacrificeItem : public
  
  desc : 희생 아이템이 내려갔을때
  arg  : none
  ret  : none
************************************************************************/
bool ZStageInterface::OnResponseCallbackSacrificeItem( const int nResult, const MUID& uidRequester, const int nSlotIndex, const int nItemID )
{
#ifdef _QUEST_ITEM
	if( MOK == nResult )
	{
		m_SacrificeItem[ nSlotIndex].RemoveItem();
		SerializeSacrificeItemListBox();

		UpdateSacrificeItem();

		MTextArea* pDesc = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_ItemDesc");
		if ( pDesc)
			pDesc->Clear();
	}
	else if( ERR_SACRIFICE_ITEM_INFO == nResult )
	{
		// 클라이언트에서 보낸 정보가 잘못된 정보. 따러 에러처리가 필요하면 여기서 해주면 됨.
	}

#endif

	return true;
}

#ifdef _QUEST_ITEM
///
// Fist : 추교성.
// Last : 추교성.
// 
// 서버로부터 QL의 정보를 받음.
///
bool ZStageInterface::OnResponseQL( const int nQL )
{
	ZGetQuest()->GetGameInfo()->SetQuestLevel( nQL);

	// 스테이지 영역에 있는 퀘스트 레벨 표시 수정
	MLabel* pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_QuestLevel");
	if ( pLabel)
	{
		char szText[125];
		sprintf_safe( szText, "%s %s : %d", ZMsg( MSG_WORD_QUEST), ZMsg( MSG_CHARINFO_LEVEL), nQL);
		pLabel->SetText( szText);
	}

	return true;
}

bool ZStageInterface::OnStageGameInfo( const int nQL, const int nMapsetID, const unsigned int nScenarioID )
{
	if (nScenarioID != 0)
	{
		ZGetQuest()->GetGameInfo()->SetQuestLevel( nQL );
	}
	else
	{
		// 시나리오가 없으면 그냥 0으로 보이게 한다.
		ZGetQuest()->GetGameInfo()->SetQuestLevel( 0 );
	}

	UpdateStageGameInfo(nQL, nMapsetID, nScenarioID);

	return true;
}

bool ZStageInterface::OnResponseSacrificeSlotInfo( const MUID& uidOwner1, const unsigned long int nItemID1, 
												   const MUID& uidOwner2, const unsigned long int nItemID2 )
{
	if ( (uidOwner1 != MUID(0,0)) && nItemID1)
	{
		MQuestItemDesc* pItemDesc = GetQuestItemDescMgr().FindQItemDesc( nItemID1);
		MBitmap* pIconBitmap = ZApplication::GetGameInterface()->GetQuestItemIcon( nItemID1, false);
		m_SacrificeItem[ SACRIFICEITEM_SLOT0].SetSacrificeItemSlot( uidOwner1, nItemID1, pIconBitmap, pItemDesc->m_szQuestItemName, pItemDesc->m_nLevel);
	}
	else
		m_SacrificeItem[ SACRIFICEITEM_SLOT0].RemoveItem();

	if ( (uidOwner2 != MUID(0,0)) && nItemID2)
	{
		MQuestItemDesc* pItemDesc = GetQuestItemDescMgr().FindQItemDesc( nItemID2);
		MBitmap* pIconBitmap = ZApplication::GetGameInterface()->GetQuestItemIcon( nItemID2, false);
		m_SacrificeItem[ SACRIFICEITEM_SLOT1].SetSacrificeItemSlot( uidOwner2, nItemID2, pIconBitmap, pItemDesc->m_szQuestItemName, pItemDesc->m_nLevel);
	}
	else
		m_SacrificeItem[ SACRIFICEITEM_SLOT1].RemoveItem();

	UpdateSacrificeItem();

	MTextArea* pDesc = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_ItemDesc");
	if ( pDesc)
		pDesc->Clear();

	return true;
}


bool ZStageInterface::OnQuestStartFailed( const int nState )
{
	MTextArea* pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "StageChattingOutput");
	if ( pTextArea)
	{
		char text[256];
		sprintf_safe(text, "^1%s", ZMsg(MSG_GANE_NO_QUEST_SCENARIO));
		pTextArea->AddText( text);
	}

/*
	if( MSQITRES_INV == nState )
	{
		// 해당 QL에대한 희생아이템 정보 테이블이 없음. 이경우는 맞지 않는 희생 아이템이 올려져 있을경우.
		MTextArea* pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "StageChattingOutput");
		if ( pTextArea)
			pTextArea->AddText( "^1현재 놓여있는 아이템은 조건에 맞지 않아 게임을 시작할 수 없습니다.");
	}
	else if( MSQITRES_DUP == nState )
	{
		// 양쪽 슬롯에 같은 아이템이 올려져 있음.
		MTextArea* pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "StageChattingOutput");
		if ( pTextArea)
			pTextArea->AddText( "^1같은 아이템 2개가 놓여있으므로 게임을 시작할 수 없습니다.");
	}
*/
	return true;
}


bool ZStageInterface::OnNotAllReady()
{
	return true;
}
#endif

void ZStageInterface::UpdateStageGameInfo(const int nQL, const int nMapsetID, const int nScenarioID)
{
	if (!ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType())) return;

	// 스테이지 영역에 있는 퀘스트 레벨 표시 수정
	MLabel* pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_QuestLevel");
	if ( pLabel)
	{
		char szText[125];
		sprintf_safe( szText, "%s %s : %d", ZMsg( MSG_WORD_QUEST), ZMsg( MSG_CHARINFO_LEVEL), nQL);
		pLabel->SetText( szText);
	}


#define		MAPSET_NORMAL		MCOLOR(0xFFFFFFFF)
#define		MAPSET_SPECIAL		MCOLOR(0xFFFFFF40)			// Green
//#define		MAPSET_SPECIAL		MCOLOR(0xFFFF2020)		// Red

	// 여기서 시나리오 이름을 보여준다.
	pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_SenarioName");
	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_SenarioNameImg");
	MPicture* pPictureL = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_Lights0");
	MPicture* pPictureR = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_Lights1");
	if ( pLabel)
	{
		if (nScenarioID == 0)
		{
			// 시나리오가 없는 경우
			pLabel->SetText( "");
			if ( pWidget)
				pWidget->Show( false);
			if ( pPictureL) {
				pPictureL->Show( false);
			}
			if ( pPictureR) {
				pPictureR->Show( false);
			}
		}
		else
		{
			pLabel->SetAlignment( MAM_HCENTER | MAM_VCENTER);

			map< int, MSenarioNameList>::iterator itr =  m_SenarioNameDesc.find( nScenarioID);
			if ( itr != m_SenarioNameDesc.end())
			{
				pLabel->SetText( (*itr).second.m_szName);
				pLabel->SetTextColor(MCOLOR(0xFFFFFF00));
				if ( pWidget)
					pWidget->Show( true);

				if ( pPictureL) {
					pPictureL->Show( true);
					pPictureL->SetBitmapColor(MAPSET_SPECIAL);
				}
				if ( pPictureR) {
					pPictureR->Show( true);
					pPictureR->SetBitmapColor(MAPSET_SPECIAL);
				}
			}
			else
			{
				// 특별시나리오가 없을경우는 정규시나리오이다.
				pLabel->SetText("");
				pLabel->SetTextColor(MCOLOR(0xFFFFFFFF));
				if ( pWidget)
					pWidget->Show( false);

				if ( pPictureL) {
					pPictureL->Show( true);
					pPictureL->SetBitmapColor(MAPSET_NORMAL);
				}
				if ( pPictureR) {
					pPictureR->Show( true);
					pPictureR->SetBitmapColor(MAPSET_NORMAL);
				}

			}
		}
	}
}

/***********************************************************************
  SetSacrificeItemSlot : public
  
  desc : 희생 아이템 슬롯에 아이템 정보를 입력
  arg  : none
  ret  : none
************************************************************************/
void SacrificeItemSlotDesc::SetSacrificeItemSlot( const MUID& uidUserID, const unsigned long int nItemID, MBitmap* pBitmap, const char* szItemName, const int nQL)
{
	m_uidUserID = uidUserID;
	m_nItemID = nItemID;
	m_pIconBitmap = pBitmap;
	strcpy_safe( m_szItemName, szItemName);
	m_nQL = nQL;
	m_bExist = true;
}


/***********************************************************************
  ReadSenarioNameXML : protected
  
  desc : 퀘스트 희생 아이템 XML을 읽는다
  arg  : none
  ret  : true(=success) or false(=fail)
************************************************************************/
bool ZStageInterface::ReadSenarioNameXML( void)
{
	m_SenarioNameDesc.clear();

	// XML 파일을 연다
	MXmlDocument xmlQuestItemDesc;
	xmlQuestItemDesc.Create();

//	if ( !xmlQuestItemDesc.LoadFromFile( "System/scenario.xml"))
//	{
//		xmlQuestItemDesc.Destroy();
//		return false;
//	}

	char			*buffer;
	MZFile			mzFile;

	if( !mzFile.Open( "System/scenario.xml", ZApplication::GetFileSystem())) 
	{
		xmlQuestItemDesc.Destroy();
		return false;
	} 

	buffer = new char[ mzFile.GetLength() + 1];
	buffer[ mzFile.GetLength()] = 0;

	mzFile.Read( buffer, mzFile.GetLength());

	if( !xmlQuestItemDesc.LoadFromMemory( buffer))
	{
		xmlQuestItemDesc.Destroy();
		return false;
	}
	delete[] buffer;
	mzFile.Close();


	// 데이터를 읽어온다
	MXmlElement rootElement = xmlQuestItemDesc.GetDocumentElement();
	for ( int i = 0;  i < rootElement.GetChildNodeCount();  i++)
	{
		MXmlElement chrElement = rootElement.GetChildNode( i);

		char szTagName[ 256];
		chrElement.GetTagName( szTagName);

		if ( szTagName[ 0] == '#')
			continue;

		bool bFindPage = false;
		if ( !_stricmp( szTagName, "SPECIAL_SCENARIO"))			// 태그 시작
		{
			char szAttrName[64];
			char szAttrValue[256];
			int nItemID = 0;
			MSenarioNameList SenarioMapList;
			
			// Set Tag
			for ( int k = 0;  k < chrElement.GetAttributeCount();  k++)
			{
				chrElement.GetAttribute( k, szAttrName, szAttrValue);

				if ( !_stricmp( szAttrName, "id"))				// ID
					nItemID = atoi( szAttrValue);

				else if ( !_stricmp( szAttrName, "title"))		// Title
					strcpy_safe( SenarioMapList.m_szName, szAttrValue);
			}

			m_SenarioNameDesc.insert( map< int, MSenarioNameList>::value_type( nItemID, SenarioMapList));
		}
	}

	xmlQuestItemDesc.Destroy();

	return true;
}

bool ZStageInterface::OnStopVote()
{
	ZGetGameClient()->SetVoteInProgress( false );
	ZGetGameClient()->SetCanVote( false );

#ifdef _DEBUG
	string str = ZMsg(MSG_VOTE_VOTE_STOP);
#endif

	ZChatOutput(ZMsg(MSG_VOTE_VOTE_STOP), ZChat::CMT_SYSTEM, ZChat::CL_CURRENT);
	return true;
}
