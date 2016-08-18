#pragma once
#include "MListBox.h"
#include "map"
#include "vector"
#include "MUID.h"
#include "ZEmblemInterface.h"
#include "ZApplication.h"

class MBitmap;
class MScrollBar;

enum ePlayerState
{
	PS_LOGOUT = 0,
	PS_FIGHT,
	PS_WAIT,
	PS_LOBBY,
	PS_END,
};

class ZPlayerListItem : public MListItem {
public:
	ZPlayerListItem() {
		m_PlayerUID = MUID(0,0);
		m_Grade = MMUG_FREE;
		m_Color = MCOLOR(0xFFCDCDCD);
	}

	void SetColor(MCOLOR c) {
		m_Color = c;
	}

	const MCOLOR GetColor(void) { 
		return m_Color; 
	}

public:

	MUID				m_PlayerUID;
	MMatchUserGradeID	m_Grade;
	MCOLOR				m_Color;

	char			m_szName[MATCHOBJECT_NAME_LENGTH];
	char			m_szClanName[CLAN_NAME_LENGTH];
	char			m_szLevel[128];

};

class ZLobbyPlayerListItem : public ZPlayerListItem{
protected:
	MBitmap* m_pBitmap;
//	MBitmap* m_pBitmapEmblem;
	unsigned int m_nClanID;

public:
	ePlayerState	m_nLobbyPlayerState;

public:
	ZLobbyPlayerListItem(const MUID& puid, MBitmap* pBitmap, unsigned int nClanID, const char* szLevel, const char* szName, const char *szClanName, ePlayerState nLobbyPlayerState,MMatchUserGradeID Grade)
	{
		m_pBitmap = pBitmap;
		m_nClanID = nClanID;
		ZGetEmblemInterface()->AddClanInfo(m_nClanID);
//		m_pBitmapEmblem = pBitmapEmblem;
		m_PlayerUID = puid;
		m_nLobbyPlayerState = nLobbyPlayerState;
		if (szLevel) strcpy_safe(m_szLevel, szLevel);
		else m_szLevel[0] = NULL;
		if (szName) strcpy_safe(m_szName, szName);
		else m_szName[0] = NULL;
		if(szClanName) strcpy_safe(m_szClanName, szClanName);
		else m_szClanName[0] = NULL;
		m_Grade = Grade;
	}

	virtual ~ZLobbyPlayerListItem() {
		ZGetEmblemInterface()->DeleteClanInfo(m_nClanID);
	}

	ZLobbyPlayerListItem(void)
	{
		m_pBitmap = NULL;
		m_nClanID = 0;
//		m_pBitmapEmblem = NULL;
		m_PlayerUID = MUID(0,0);
		m_nLobbyPlayerState = (ePlayerState)0;
		m_szLevel[0] = 0;
		m_szName[0] = 0;
		m_szClanName[0] = 0;
		m_Grade = MMUG_FREE;
	}

	virtual const char* GetString(void)
	{
		return m_szName;
	}

	virtual const char* GetString(int i)
	{
		if(i==1) return m_szLevel;
		else if(i==2) return m_szName;
		else if(i==4) return m_szClanName;
		return NULL;
	}

	virtual MBitmap* GetBitmap(int i)
	{
		if (i == 0) return m_pBitmap;
		else if (i == 3)
		{
			if ( strcmp( m_szClanName, "") == 0)
				return NULL;
			else
				return ZGetEmblemInterface()->GetClanEmblem(m_nClanID);
		}
		return NULL;
	}

	MUID& GetUID() { return m_PlayerUID; }
};

class ZFriendPlayerListItem : public ZPlayerListItem{
protected:
	char		m_szLocation[128];
	MBitmap*	m_pBitmap;

public:
	ePlayerState	m_nLobbyPlayerState;

public:
	ZFriendPlayerListItem(const MUID& puid, MBitmap* pBitmap, const char* szName, const char* szClanName,const char* szLocation, ePlayerState nLobbyPlayerState,MMatchUserGradeID Grade)
	{
		m_pBitmap = pBitmap;
		m_PlayerUID = puid;
		m_nLobbyPlayerState = nLobbyPlayerState;
		if (szName) strcpy_safe(m_szName, szName);
		else m_szName[0] = NULL;
		if(szClanName) strcpy_safe(m_szClanName, szClanName);
		else m_szClanName[0] = NULL;

		m_szLevel[0] = NULL;

		if (szLocation)
			strcpy_safe(m_szLocation, szLocation);
		else
			m_szLocation[0] = NULL;

		m_Grade = Grade;
	}

	ZFriendPlayerListItem(void)
	{
		m_pBitmap = NULL;
		m_PlayerUID = MUID(0,0);
		m_nLobbyPlayerState = (ePlayerState)0;
		m_szName[0] = PS_END;
		m_szLevel[0] = NULL;
		m_szLocation[0] = NULL;

		m_Grade = MMUG_FREE;
	}

	virtual const char* GetString(void)
	{
		return m_szName;
	}

	virtual const char* GetString(int i)
	{
		if(i==1) return m_szName;
		//else if(i==2) return m_szLocation;
		return NULL;
	}

	virtual MBitmap* GetBitmap(int i)
	{
		if (i == 0) return m_pBitmap;
		return NULL;
	}

	MUID& GetUID() { return m_PlayerUID; }
	const char* GetLocation() { return m_szLocation; }
};

class ZClanPlayerListItem : public ZPlayerListItem{
protected:
	MBitmap* m_pBitmap;
	MMatchClanGrade	m_ClanGrade;

public:
	ePlayerState	m_nLobbyPlayerState;

public:
	ZClanPlayerListItem(const MUID& puid, MBitmap* pBitmap, const char* szName, const char* szClanName,const char* szLevel, ePlayerState nLobbyPlayerState,MMatchClanGrade clanGrade)
	{
		m_pBitmap = pBitmap;
		m_PlayerUID = puid;
		m_nLobbyPlayerState = nLobbyPlayerState;
		if (szName)
			strcpy_safe(m_szName, szName);
		else
			m_szName[0] = NULL;
		if (szLevel) strcpy_safe(m_szLevel, szLevel);
		else m_szLevel[0] = NULL;
		if(szClanName) strcpy_safe(m_szClanName, szClanName);
		else m_szClanName[0] = NULL;

		m_ClanGrade = clanGrade;
		m_Color = MCOLOR(0xFFCDCDCD);
	}

	ZClanPlayerListItem(void)
	{
		m_pBitmap = NULL;
		m_PlayerUID = MUID(0,0);
		m_nLobbyPlayerState = (ePlayerState)0;
		m_szName[0] = PS_END;
		m_szClanName[0] = PS_END;
		m_szLevel[0] = 0;
		m_Grade = MMUG_FREE;
		m_ClanGrade = MCG_NONE;
		m_Color = MCOLOR(0xFFCDCDCD);
	}

	virtual const char* GetString(void)
	{
		return m_szName;
	}

	virtual const char* GetString(int i)
	{
		if(i==1) return m_szName;
		else if(i==3) return m_szClanName;
		return NULL;
	}

	virtual MBitmap* GetBitmap(int i)
	{
		if (i == 0) return m_pBitmap;
		return NULL;
	}

	MUID& GetUID() { return m_PlayerUID; }
};

enum eStagePlayerState
{
	SPS_NONE = 0,
	SPS_SHOP,
	SPS_EQUIP,
	SPS_READY,
	SPS_END
};

struct sStagePlayerInfo
{
	int Level;
	char szName[128];
	int state;
	bool isMaster;
	int	nTeam;
};


class ZStagePlayerListItem : public ZPlayerListItem{
public:
	int		m_nTeam;
	bool	m_bEnableObserver;			// 옵져버인지 아닌지... : 동환이가 추가
	unsigned int m_nClanID;

public:
	MBitmap* m_pBitmap;
//	MBitmap* m_pBitmapEmblem;

public:
	ZStagePlayerListItem(const MUID& puid, MBitmap* pBitmap, unsigned int nClanID, const char* szName, const char* szClanName, const char* szLevel,MMatchUserGradeID Grade)
	{
		m_pBitmap = pBitmap;
		m_nClanID = nClanID;
		ZGetEmblemInterface()->AddClanInfo(m_nClanID);
//		m_pBitmapEmblem = pBitmapEmblem;
		m_PlayerUID = puid;
		strcpy_safe(m_szName, szName);
		if(szClanName) strcpy_safe(m_szClanName, szClanName);
		else m_szClanName[0] = NULL;

		strcpy_safe(m_szLevel, szLevel);
		m_nTeam = 0;
		m_Grade = Grade;
	}

	virtual ~ZStagePlayerListItem()
	{
		ZGetEmblemInterface()->DeleteClanInfo(m_nClanID);
	}

	ZStagePlayerListItem(void)
	{
		m_pBitmap = NULL;
		m_nClanID = 0;
//		m_pBitmapEmblem = NULL;
		m_PlayerUID = MUID(0,0);
		m_szName[0] = 0;
		m_szLevel[0] = 0;
		m_Grade = MMUG_FREE;
	}

	virtual const char* GetString(void)
	{
		return m_szName;
	}

	virtual const char* GetString(int i)
	{
		if(i==1) return m_szLevel;
		else if(i==2) return m_szName;
		else if(i==4) return m_szClanName;
		return NULL;
	}

	virtual MBitmap* GetBitmap(int i)
	{
		if (i == 0) return m_pBitmap;
		else if (i == 3)
		{
			if ( strcmp( m_szClanName, "") == 0)
				return NULL;
			else
				return ZGetEmblemInterface()->GetClanEmblem(m_nClanID);
		}
		return NULL;
	}

	MUID& GetUID() { return m_PlayerUID; }
};

class ZPlayerListBoxLook : public MListBoxLook
{
public:
	virtual void OnItemDraw2(MDrawContext* pDC, MRECT& r, const char* szText, MCOLOR color, bool bSelected, bool bFocus, int nAdjustWidth = 0);
	virtual void OnItemDraw2(MDrawContext* pDC, MRECT& r, MBitmap* pBitmap, bool bSelected, bool bFocus, int nAdjustWidth);
	virtual void OnDraw(MListBox* pListBox, MDrawContext* pDC);

	virtual MRECT GetClientRect(MListBox* pListBox, const MRECT& r);
};

class ZPlayerListBox : public MListBox
{
public:
	enum PLAYERLISTMODE {
		PLAYERLISTMODE_CHANNEL = 0,
		PLAYERLISTMODE_STAGE ,
		PLAYERLISTMODE_CHANNEL_FRIEND ,
		PLAYERLISTMODE_STAGE_FRIEND ,
		PLAYERLISTMODE_CHANNEL_CLAN ,
		PLAYERLISTMODE_STAGE_CLAN ,

		PLAYERLISTMODE_END,	// = count
	};

private:
//	MBitmap*					m_pBitmap;
//	MBitmap*					m_pBitmapIn;

	MBmButton*					m_pButton;

//	map< MUID, sPlayerInfo*>	mPlayers;
	vector<MUID>				mPlayerOrder;

	int				mSelectedPlayer;
	int				mStartToDisplay;
	float			m_SlotWidth;
	float			m_SlotHeight;

	int				m_nOldW;
	PLAYERLISTMODE		m_nMode;


public:

	

protected:
	void SetupButton(const char *szOn, const char *szOff);

public:
	void InitUI(PLAYERLISTMODE nMode);
	void RefreshUI();

	PLAYERLISTMODE GetMode() { return m_nMode; }
	void SetMode(PLAYERLISTMODE mode);

//	void SetBitmap( MBitmap* pBitmap );
//	MBitmap* GetBitmap() {	return m_pBitmap; }
//	MBitmap* GetBitmapIn() { return m_pBitmapIn; }

	// mode PLAYERLISTMODE_CHANNEL
	void AddPlayer(MUID& puid, ePlayerState state, int  nLevel,char* szName, char* szClanName, unsigned int nClanID, MMatchUserGradeID nGrade );

	// mode PLAYERLISTMODE_STAGE
	void AddPlayer(MUID& puid, MMatchObjectStageState state, int nLevel, char* szName, char* szClanName, unsigned int nClanID, bool isMaster,MMatchTeam nTeam);

	// mode PLAYERLISTMODE_CHANNEL_FRIEND, PLAYERLISTMODE_STAGE_FRIEND
	void AddPlayer(ePlayerState state, char* szName, char* szLocation);

	// mode PLAYERLISTMODE_CHANNEL_CLAN
	void AddPlayer(MUID& puid, ePlayerState state, char* szName, int  nLevel ,MMatchClanGrade nGrade );

	void DelPlayer(MUID& puid);
	void UpdatePlayer(MUID& puid,MMatchObjectStageState state, char* szName, int  nLevel ,bool isMaster,MMatchTeam nTeam);
	void UpdatePlayer(MUID& puid,MMatchObjectStageState state, bool isMaster,MMatchTeam nTeam);
	void UpdateEmblem(MUID& puid);

	void UpdateList(int mode);

	ZPlayerListItem* GetUID(MUID uid);
	const char* GetPlayerName( int nIndex);

	MUID GetSelectedPlayerUID();
	void SelectPlayer(MUID);

	virtual bool OnEvent(MEvent* pEvent, MListener* pListener);
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage);

//	MUID	m_MyUID;
//	MUID	m_uidChannel;
	int		m_nTotalPlayerCount;
	int		m_nPage;

	void SetWidth( float t ) { m_SlotWidth = t;	}
	void SetHeight( float t ) { m_SlotHeight = t; }

	PLAYERLISTMODE GetPlayerListMode()		{ return m_nMode; }
public:
	ZPlayerListBox(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~ZPlayerListBox(void);

	DECLARE_LOOK(ZPlayerListBoxLook)
	DECLARE_LOOK_CLIENT()

	void OnSize(int w,int h);
};

////////////////////////////////////////////////////////////////////////////////////////

/*
class ZStagePlayerListBox : public MListBox
{
private:
	MBitmap*		m_pBitmap;

	map< MUID, sStagePlayerInfo*>	mPlayers;
	vector<MUID>					mPlayerOrder;

	int				mSelectedPlayer;
	int				mStartToDisplay;
	float			m_SlotWidth;
	float			m_SlotHeight;

	int				m_nOldW;
protected:

public:
	void SetBitmap( MBitmap* pBitmap );
	MBitmap* GetBitmap() {	return m_pBitmap; }

	void AddPlayer(MMatchObjCache* pCache);
	void AddPlayer(MUID& puid, MMatchObjectStageState state, char* szName, int  nLevel ,bool isMaster,MMatchTeam nTeam);
//	void AddPlayer(MUID& puid, eStagePlayerState state, char* szName, int  nLevel ,bool isMaster,int nTeam);
	void DelPlayer(MUID& puid);
	void UpdatePlayer(MUID& puid,eStagePlayerState state, char* szName, int  nLevel ,bool isMaster,int nTeam);

	ZStagePlayerListItem* GetUID(MUID uid);

	MUID	m_MyUID;
	MUID	m_uidChannel;
	int		m_nTotalPlayerCount;
	int		m_nPage;

	virtual bool OnEvent(MEvent* pEvent, MListener* pListener);


	void SetWidth( float t ) { m_SlotWidth = t;	}
	void SetHeight( float t ) { m_SlotHeight = t; }

	void Resize(float x,float y);

	float OnReSize();

public:
	ZStagePlayerListBox(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~ZStagePlayerListBox(void);

	DECLARE_LOOK(ZPlayerListBoxLook)
	DECLARE_LOOK_CLIENT()
};
*/