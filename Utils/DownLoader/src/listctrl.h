#pragma once

// ListCtrl

#define TLT_TEXT		0
#define TLT_PROGRESSBAR	1

#define ListItem ListCtrl::Item *

class ListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(ListCtrl)
	DECLARE_MESSAGE_MAP()

protected:
	struct Column
	{
		bool m_Enabled;
		DWORD m_Width;
		DWORD m_Id;
		CString m_Text;
	};

	struct ItemData
	{
		DWORD m_Id;
		CWnd * m_Control;
		bool m_ControlVisible;
		CRect m_ControlRect;
		CString m_Text;
	};

public:
	struct Item
	{
		~Item(void)
		{
			while(m_Data.GetSize())
			{
				MFree(m_Data[0]);
				m_Data.Delete(0);
			}
		}

		DWORD m_Image;
		DWORD_PTR m_CustomData;
		List<ItemData *> m_Data;
	};
public:
	ListCtrl();
	virtual ~ListCtrl();

	bool Create(DWORD dwStyle, CRect &Rect, CWnd * ParentWnd, UINT nId);

	// Window Messages
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	// Interface
	void AddColumn(const char * Text, DWORD Width, DWORD Id);
//	void ShowColumn(DWORD Id, bool Visible);

	ListItem InsertItem(int Index);
	ListItem AddItem(void);
	void DeleteItem(ListItem Item);
	
	void SetItemText(ListItem Item, const char * Text, DWORD Id);
	void SetItemImage(ListItem Item, DWORD Image);
	void SetItemControl(ListItem Item, CWnd * Control, DWORD Id);

//	void AddImage(DWORD ResourceId);

	POSITION GetFirstSelectedItemPosition(void);
	ListItem GetNextSelectedItem(POSITION & Pos);

	void SetItemData(ListItem Item, DWORD_PTR Data);
	DWORD_PTR GetItemData(ListItem Item);

	void SetActiveFlags(DWORD f);

protected:
	int ListItemToIndex(ListItem Item);
	ItemData * IdToItemData(ListItem Item, DWORD Id, bool CreateIfNotExist);
	void UpdateControls(bool Invalidate);
	int IdToCol(DWORD Id);
	void UpdateListData(void);

	CWnd * m_ParentWnd;
	
	List<Item *> m_Items;
	List<Column *> m_Columns;

//	CBitmap m_Images;
//	CImageList m_Images;
public:
	afx_msg void OnPaint();
	afx_msg void OnHdnItemchanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
};