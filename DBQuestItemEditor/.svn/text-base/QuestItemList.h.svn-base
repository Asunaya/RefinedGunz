#pragma once


// CQuestItemList


#include <vector>
#include <map>
#include <string>
using namespace std;

#include "MQuestItem.h"
#include "MMatchObject.h"
#include "MMatchDBMgr.h"
#include "MZFileSystem.h"
#include "MBaseStringResManager.h"
#include "MMatchStringResManager.h"


// Below styles MUST be present in a report ctrl
#define MUST_STYLE			(LVS_REPORT | LVS_SHOWSELALWAYS)
#define MUST_EX_STYLE		(LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES)

// Below styles MUST NOT be present in a report ctrl
#define MUST_NOT_STYLE		(LVS_EDITLABELS | LVS_ICON | LVS_SMALLICON | LVS_LIST | LVS_NOSCROLL)
#define MUST_NOT_EX_STYLE	(0)


#define CONFIG_FILE				"config.ini"
#define QUEST_ITEM_XML_FILE		"zquestitem.xml"

struct QItem
{
	QItem( const long nQIID, const string strName, const int nCount ) :
		m_nQIID( nQIID ), m_strName( strName ), m_nCount( nCount ) {}

	long	m_nQIID;
	string	m_strName;
	int		m_nCount;
};


typedef vector< QItem > QItemHistory;


class CQuestItemList : public CListCtrl
{
	DECLARE_DYNAMIC(CQuestItemList)

public:
	CQuestItemList();
	virtual ~CQuestItemList();

	bool Init();
	bool LoadQuestItemXML();
	void InitListColumn();
	void MakeQItemInfoColumns();
	bool GetDBQItemInfo( const string strCharName );
	bool GetDBQItemInfo( const int nCID );
	void UpdateDB();
	void EditQItem( const long nQIID, const int nCount );
	void Clear(); // 이전에 설정된 모든 데이터를 초기화 함.

	const int FindCIDByCharName( const string& strCharName );
	const string FindCharNameByCID( const int nCID );

private :
	bool ConnectDB();
	bool DisconnectDB();
	
private :
	MMatchDBMgr				m_DBMgr;
	MMatchCharInfo			m_MCharInfo;
	MQuestItemDescManager	m_MQItemDescMgr;
	QItemHistory			m_QItemEditHistory;

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
};


