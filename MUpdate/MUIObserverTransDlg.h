#include "MUIObserver.h"


class CTransferDlg;


class MUIObserverTransDlg : public MUIObserver {
	CTransferDlg*	m_pDlg;

public:
	MUIObserverTransDlg(CTransferDlg* pDlg)		{	m_pDlg = pDlg;	}
	virtual ~MUIObserverTransDlg()				{}

	virtual void UpdateMsg(int nCategory, char* pszMsg);
	virtual void UpdateProgress(char* pszFileName, unsigned long nRead, unsigned long nTransBytes, unsigned long nMaxBytes);
};
