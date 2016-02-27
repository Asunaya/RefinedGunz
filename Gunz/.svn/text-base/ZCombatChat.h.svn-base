#ifndef _ZCOMBATCHAT_H
#define _ZCOMBATCHAT_H

#include "ZInterface.h"

//class MListBox;
class MEdit;
class MTextArea;

/////////////////
// ZTabPlayerList
#define ZPLB_ITEM_PICKPLAYER	"picked"
class ZTabPlayerList : public MListBox {
protected:
	MEdit*	m_pEditChat;

public:
	ZTabPlayerList(const char* szName, MWidget* pParent=NULL, MListener* pListener=NULL);
	void SetChatControl(MEdit* pEdit)	{ m_pEditChat = pEdit; }

	virtual bool OnShow(void);
	virtual void OnHide(void);
	virtual bool OnEvent(MEvent* pEvent, MListener* pListener);

	void OnPickPlayer();
};


class ZCombatChat
{
private:
protected:
	ZIDLResource*		m_pIDLResource;
	MEdit*				m_pInputEdit;
	ZTabPlayerList*		m_pTabPlayerList;
	bool				m_bChatInputVisible;
	unsigned long int	m_nLastChattingMsgTime;
	bool				m_bTeamChat;
	bool				m_bShowOutput;

	void SetTeamChat(bool bVal)	{ m_bTeamChat = bVal; }
	void UpdateChattingBox();
	void ProcessChatMsg();
public:
	MTextArea*			m_pChattingOutput;

	ZCombatChat();
	virtual ~ZCombatChat();
	bool Create( const char* szOutputTxtarea,bool bUsePlayerList);
	void Destroy();

	void Update();
	void EnableInput(bool bEnable, bool bToTeam=false);
	void OutputChatMsg(const char* szMsg);
	void OutputChatMsg(MCOLOR color, const char* szMsg);

	void OnDraw(MDrawContext* pDC);
	bool IsChat() { return m_bChatInputVisible; }
	bool IsTeamChat()	{ return m_bTeamChat; }
	bool IsShow()
	{
		if (m_pInputEdit == NULL) return false;
		return m_pInputEdit->IsVisible();
	}
	void SetFont( MFont* pFont);

	void ShowOutput(bool bShow);
};




#endif