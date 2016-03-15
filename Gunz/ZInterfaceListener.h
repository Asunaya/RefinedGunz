#ifndef ZINTERFACELISTENER_H
#define ZINTERFACELISTENER_H

class MListener;

// 리스너 선언 매크로
#define DECLARE_LISTENER(_FunctionName)	MListener* _FunctionName(void);

/// 리스너 정의 매크로, BEGIN_ ~ END_ 사이에 원하는 코드를 넣는다.
#define BEGIN_IMPLEMENT_LISTENER(_FunctionName, _szMessageName)					\
	MListener* _FunctionName(void){										\
class ListenerClass : public MListener{									\
public:																	\
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){	\
	if(MWidget::IsMsg(szMessage, _szMessageName)==true){

#define END_IMPLEMENT_LISTENER()			\
	return true;			\
	}							\
	return false;				\
	}								\
};									\
	static ListenerClass	Listener;	\
	return &Listener;					\
	}


// Gunz에 쓰이는 전역 Listener Object 얻기
MListener* ZGetChatInputListener(void);
MListener* ZGetLoginListener(void);
MListener* ZGetCreateAccountFrameCallerListener(void);
MListener* ZGetCreateAccountBtnListener(void);
MListener* ZGetLogoutListener(void);
MListener* ZGetExitListener(void);
MListener* ZGetChannelChatInputListener(void);
MListener* ZGetStageChatInputListener(void);
MListener* ZGetGameStartListener(void);
MListener* ZGetMapChangeListener(void);
MListener* ZGetMapSelectListener(void);
MListener* ZGetParentCloseListener(void);
//MListener* ZGetCombatChatInputListener(void);
MListener* ZGetStageCreateFrameCallerListener(void);
MListener* ZGetSelectCharacterComboBoxListener(void);


// 로그인
DECLARE_LISTENER(ZGetLoginStateButtonListener)
DECLARE_LISTENER(ZGetGreeterStateButtonListener)
DECLARE_LISTENER(ZGetOptionFrameButtonListener)
DECLARE_LISTENER(ZGetRegisterListener)

// 로비
/*
DECLARE_LISTENER(ZGetChannelPlayerOptionGroupAll)
DECLARE_LISTENER(ZGetChannelPlayerOptionGroupFriend)
DECLARE_LISTENER(ZGetChannelPlayerOptionGroupClan)
*/
//DECLARE_LISTENER(ZGetLobbyStageListListener)
DECLARE_LISTENER(ZGetLobbyPrevRoomListButtonListener)
DECLARE_LISTENER(ZGetLobbyNextRoomListPrevButtonListener)

/*
DECLARE_LISTENER(ZGetLobbyNextRoomNo1ButtonListener)
DECLARE_LISTENER(ZGetLobbyNextRoomNo2ButtonListener)
DECLARE_LISTENER(ZGetLobbyNextRoomNo3ButtonListener)
DECLARE_LISTENER(ZGetLobbyNextRoomNo4ButtonListener)
DECLARE_LISTENER(ZGetLobbyNextRoomNo5ButtonListener)
DECLARE_LISTENER(ZGetLobbyNextRoomNo6ButtonListener)
*/
DECLARE_LISTENER(ZGetLobbyNextRoomNoButtonListener)

/*
DECLARE_LISTENER(ZGetListenerLobbyPlayerListTabChannel);
DECLARE_LISTENER(ZGetListenerLobbyPlayerListTabFriend);
DECLARE_LISTENER(ZGetListenerLobbyPlayerListTabClan);
*/
DECLARE_LISTENER(ZGetPlayerListPrevListener);
DECLARE_LISTENER(ZGetPlayerListNextListener);

DECLARE_LISTENER(ZGetArrangedTeamGameListener);
DECLARE_LISTENER(ZGetArrangedTeamDialogOkListener);
DECLARE_LISTENER(ZGetArrangedTeamDialogCloseListener);
DECLARE_LISTENER(ZGetArrangedTeamGame_CancelListener);

DECLARE_LISTENER(ZGetLeaveClanOKListener);
DECLARE_LISTENER(ZGetLeaveClanCancelListener);

// Channel frame
DECLARE_LISTENER(ZGetChannelListJoinButtonListener)
DECLARE_LISTENER(ZGetChannelListCloseButtonListener)
DECLARE_LISTENER(ZGetChannelListListener)
DECLARE_LISTENER(ZGetPrivateChannelEnterListener);
DECLARE_LISTENER(ZGetMyClanChannel);
DECLARE_LISTENER(ZGetChannelList);
DECLARE_LISTENER(ZGetMyClanChannel);

// Stage
DECLARE_LISTENER(ZGetMapListListener)
DECLARE_LISTENER(ZGetStageListFrameCallerListener)
DECLARE_LISTENER(ZGetStageCreateBtnListener)
DECLARE_LISTENER(ZGetPrivateStageJoinBtnListener)
DECLARE_LISTENER(ZGetChannelListFrameCallerListener)
DECLARE_LISTENER(ZGetStageJoinListener)
DECLARE_LISTENER(ZGetStageSettingCallerListener)
DECLARE_LISTENER(ZGetStageSettingStageTypeListener)
DECLARE_LISTENER(ZGetStageSettingChangedComboboxListener)			// 동환이가 작성
DECLARE_LISTENER(ZGetStageTeamRedListener)
DECLARE_LISTENER(ZGetStageTeamBlueListener)
DECLARE_LISTENER(ZGetStageObserverBtnListener)
DECLARE_LISTENER(ZGetStageReadyListener)
DECLARE_LISTENER(ZGetStageSettingApplyBtnListener)
DECLARE_LISTENER(ZGetLobbyListener)
DECLARE_LISTENER(ZGetBattleExitButtonListener)
DECLARE_LISTENER(ZGetStageExitButtonListener)
DECLARE_LISTENER(ZGetCombatMenuCloseButtonListener)
DECLARE_LISTENER(ZGetPreviousStateButtonListener)
DECLARE_LISTENER(ZGetShopCallerButtonListener)
DECLARE_LISTENER(ZGetShopCloseButtonListener)
DECLARE_LISTENER(ZGetQuickJoinButtonListener)
DECLARE_LISTENER(ZGetLobbyCharInfoCallerButtonListener)
DECLARE_LISTENER(ZGetStageMapListSelectionListener)
//DECLARE_LISTENER(ZGetStageMapListCallerListener)
DECLARE_LISTENER(ZStageSacrificeItem0)
DECLARE_LISTENER(ZStageSacrificeItem1)
DECLARE_LISTENER(ZStagePutSacrificeItem)
DECLARE_LISTENER(ZStageSacrificeItemBoxOpen)
DECLARE_LISTENER(ZStageSacrificeItemBoxClose)


DECLARE_LISTENER(ZGetStagePlayerListPrevListener);
DECLARE_LISTENER(ZGetStagePlayerListNextListener);

// 옵션들은 ZOptionInterface 로 옮김

// 옵션
// 옵션들은 ZOptionInterface 로 옮김

// 상점
DECLARE_LISTENER(ZGetEquipmentCallerButtonListener)
DECLARE_LISTENER(ZGetEquipmentCloseButtonListener)
DECLARE_LISTENER(ZGetCharSelectionCallerButtonListener)
DECLARE_LISTENER(ZGetSellButtonListener)
DECLARE_LISTENER(ZGetSellQuestItemConfirmOpenListener)
DECLARE_LISTENER(ZGetSellQuestItemConfirmCloseListener)
DECLARE_LISTENER(ZGetSellQuestItemButtonListener)
DECLARE_LISTENER(ZGetItemCountUpButtonListener)
DECLARE_LISTENER(ZGetItemCountDnButtonListener)
DECLARE_LISTENER(ZGetBuyButtonListener)
DECLARE_LISTENER(ZGetBuyCashItemButtonListener)
DECLARE_LISTENER(ZGetEquipButtonListener)
DECLARE_LISTENER(ZGetEquipmentSearchButtonListener)
DECLARE_LISTENER(ZGetStageForcedEntryToGameListener)
DECLARE_LISTENER(ZGetAllEquipmentListCallerButtonListener)			// 구입버튼
DECLARE_LISTENER(ZGetMyAllEquipmentListCallerButtonListener)		// 판매버튼
DECLARE_LISTENER(ZGetCashEquipmentListCallerButtonListener)			// 캐쉬샵 버튼
DECLARE_LISTENER(ZGetShopEquipmentCallerButtonListener)				// 샾->장비 버튼 선택
DECLARE_LISTENER(ZGetSendAccountItemButtonListener)					// 보내기
DECLARE_LISTENER(ZGetBringAccountItemButtonListener)				// 가져오기
DECLARE_LISTENER(ZShopListFrameClose)								// ListFrame 닫기 버튼
DECLARE_LISTENER(ZShopListFrameOpen)								// ListFrame 열기 버튼

//DECLARE_LISTENER(ZGetShopAskButtonListener);						// 조르기
//DECLARE_LISTENER(ZGetShopGiftButtonListener);						// 선물하기
DECLARE_LISTENER(ZGetShopCachRechargeButtonListener);				// 충전하기
DECLARE_LISTENER(ZGetShopSearchCallerButtonListener);				// 검색

// 장비
DECLARE_LISTENER(ZGetEquipmentCharacterTabButtonListener);			// 캐릭터 탭 선택
DECLARE_LISTENER(ZGetEquipmentAccountTabButtonListener);			// 중앙은행 탭 선택
DECLARE_LISTENER(ZGetEquipmentShopCallerButtonListener);			// 장비 -> 샾 버튼 선택
DECLARE_LISTENER(ZGetLevelConfirmListenter);						// 중앙은행 레벨무시 가져오기 확인
DECLARE_LISTENER(ZEquipListFrameClose)								// ListFrame 닫기 버튼
DECLARE_LISTENER(ZEquipListFrameOpen)								// ListFrame 열기 버튼
DECLARE_LISTENER(ZEquipmetRotateBtn)								// 캐릭터 뷰 회전 변경 버튼


// 캐릭터 선택 관련
DECLARE_LISTENER(ZGetSelectCharacterButtonListener);
DECLARE_LISTENER(ZGetShowCreateCharacterButtonListener);
DECLARE_LISTENER(ZGetDeleteCharacterButtonListener);
DECLARE_LISTENER(ZGetConfirmDeleteCharacterButtonListener);
DECLARE_LISTENER(ZGetCloseConfirmDeleteCharButtonListener);

DECLARE_LISTENER(ZGetSelectCharacterButtonListener0);
DECLARE_LISTENER(ZGetSelectCharacterButtonListener1);
DECLARE_LISTENER(ZGetSelectCharacterButtonListener2);
DECLARE_LISTENER(ZGetSelectCharacterButtonListener3);

DECLARE_LISTENER(ZGetShowCharInfoGroupListener);
DECLARE_LISTENER(ZGetShowEquipInfoGroupListener);

// 맵선택 관련
DECLARE_LISTENER(ZGetMapComboListener);
DECLARE_LISTENER(ZGetSelectMapPrevButtonListener);
DECLARE_LISTENER(ZGetSelectMapNextButtonListener);

// 선택된 캐릭터 돌리기
DECLARE_LISTENER(ZGetSelectCameraLeftButtonListener);
DECLARE_LISTENER(ZGetSelectCameraRightButtonListener);

// 캐릭터 생성 화면
DECLARE_LISTENER(ZGetCreateCharacterButtonListener);
DECLARE_LISTENER(ZGetCancelCreateCharacterButtonListener);
DECLARE_LISTENER(ZChangeCreateCharInfoListener);
DECLARE_LISTENER(ZGetCreateCharacterLeftButtonListener);
DECLARE_LISTENER(ZGetCreateCharacterRightButtonListener);

// 리플레이 브라우져 관련
DECLARE_LISTENER(ZReplayOk);
DECLARE_LISTENER(ZGetReplayCallerButtonListener);
DECLARE_LISTENER(ZGetReplayViewButtonListener);
DECLARE_LISTENER(ZGetReplayExitButtonListener);
DECLARE_LISTENER(ZGetReplayFileListBoxListener);


// 게임 종료 관련
DECLARE_LISTENER(ZGetGameResultQuit);


// 몬스터 도감
DECLARE_LISTENER(ZGetMonsterInterfacePrevPage);
DECLARE_LISTENER(ZGetMonsterInterfaceNextPage);
DECLARE_LISTENER(ZGetMonsterInterfaceQuit);


// 기타
//DECLARE_LISTENER(ZGetListenerGamePlayerListTabGame);
//DECLARE_LISTENER(ZGetListenerGamePlayerListTabFriend);
DECLARE_LISTENER(ZGetRoomListListener);
DECLARE_LISTENER(ZGetMonsterBookCaller);


// 112 확인창
DECLARE_LISTENER(ZGet112ConfirmEditListener);
DECLARE_LISTENER(ZGet112ConfirmOKButtonListener);
DECLARE_LISTENER(ZGet112ConfirmCancelButtonListener);

// 클랜 관련
DECLARE_LISTENER(ZGetClanSponsorAgreementConfirm_OKButtonListener);
DECLARE_LISTENER(ZGetClanSponsorAgreementConfirm_CancelButtonListener);
DECLARE_LISTENER(ZGetClanSponsorAgreementWait_CancelButtonListener);
DECLARE_LISTENER(ZGetClanJoinerAgreementConfirm_OKButtonListener);
DECLARE_LISTENER(ZGetClanJoinerAgreementConfirm_CancelButtonListener);
DECLARE_LISTENER(ZGetClanJoinerAgreementWait_CancelButtonListener);

// 클랜 생성 다이얼로그
DECLARE_LISTENER(ZGetLobbyPlayerListTabClanCreateButtonListener);
DECLARE_LISTENER(ZGetClanCreateDialogOk);
DECLARE_LISTENER(ZGetClanCreateDialogClose);

// 확인 다이알로그
DECLARE_LISTENER(ZGetClanCloseConfirmListenter);
DECLARE_LISTENER(ZGetClanLeaveConfirmListenter);

// 동의요청 취소 버튼
DECLARE_LISTENER(ZGetProposalAgreementWait_CancelButtonListener);
DECLARE_LISTENER(ZGetProposalAgreementConfirm_OKButtonListener);
DECLARE_LISTENER(ZGetProposalAgreementConfirm_CancelButtonListener);
// 동의승락



#endif
