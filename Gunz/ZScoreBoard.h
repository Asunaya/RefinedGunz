#ifndef _ZSCOREBOARD_H
#define _ZSCOREBOARD_H

#include "ZInterface.h"
#include "ZCharacterManager.h"


class MFrame;
class ZFrame;
class ZScoreListBox;

class ZScoreBoard : public ZInterface
{
private:
protected:
	MFrame*				m_pFrame;
	ZScoreListBox*		m_pListBox;
	ZCharacterManager*	m_pCharacterManager;
	ZIDLResource*		m_pIDLResource;
	virtual bool OnShow(void);
	virtual void OnHide(void);

	void Refresh();
	void Clear();
	void Add(const char* szTeam, const char* szID, const char* szState, 
		int nScore, int nKills, int nDeaths, int nPing);
public:
	ZScoreBoard(ZCharacterManager* pCharacterManager, ZIDLResource* pIDLResource,
		        const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~ZScoreBoard();
	virtual bool OnCreate();
	virtual void OnDestroy();

	virtual void OnDraw(MDrawContext* pDC);
	virtual bool OnEvent(MEvent* pEvent, MListener* pListener);

	void Update();
};


#endif