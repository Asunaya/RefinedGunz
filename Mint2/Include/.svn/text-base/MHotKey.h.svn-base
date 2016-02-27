#ifndef MHOTKEY_H
#define MHOTKEY_H

#include "MEdit.h"

/// HotKey
class MHotKey : public MEdit{
protected:
	DECLARE_LOOK(MEditLook)	// Edit Look을 그대로 쓴다.
	DECLARE_LOOK_CLIENT()

protected:
	unsigned int	m_nKey;
	bool			m_bCtrl;
	bool			m_bAlt;
	bool			m_bShift;

protected:
	virtual void OnRun(void);
	virtual bool OnEvent(MEvent* pEvent, MListener* pListener);

public:
	MHotKey(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);

#define MINT_HOTKEY	"HotKey"
	virtual const char* GetClassName(void){ return MINT_HOTKEY; }

	void GetHotKeyName(char* szHotKeyName);
	void GetHotKey(unsigned int* pKey, bool* pCtrl, bool* pAlt, bool* pShift);

	int RegisterHotKey(void);
	void UnregisterHotKey(int nID);
};


#endif