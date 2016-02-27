#pragma once
#include "afxcmn.h"

#include <vector>
#include <string>

using std::vector;
using std::string;

// MServerInfoDlg 대화 상자입니다.

class MServerInfoDlg : public CDialog
{
	DECLARE_DYNAMIC(MServerInfoDlg)

public:
	MServerInfoDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~MServerInfoDlg();

	void Init();
	void Reset();

private :
	void InitColumnName();
		
// 대화 상자 데이터입니다.
	enum { IDD = IDD_SERVER_INFO_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

private:
	CListCtrl			m_ServerInfo;
	vector< string >	m_vColumnName;
};