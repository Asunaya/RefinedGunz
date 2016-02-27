// ListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Downloader.h"
#include "listctrl.h"
#include ".\listctrl.h"

// ListCtrl

IMPLEMENT_DYNAMIC(ListCtrl, CListCtrl)
BEGIN_MESSAGE_MAP(ListCtrl, CListCtrl)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_SETCURSOR()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_KILLFOCUS()
	ON_WM_LBUTTONDOWN()
	ON_WM_CREATE()
	ON_WM_VSCROLL()
	ON_WM_ACTIVATE()
	ON_WM_CHILDACTIVATE()
	ON_WM_PAINT()
	ON_NOTIFY(HDN_ITEMCHANGEDA, 0, OnHdnItemchanged)
	ON_NOTIFY(HDN_ITEMCHANGEDW, 0, OnHdnItemchanged)
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

ListCtrl::ListCtrl()
{
	m_ParentWnd = NULL;
}

ListCtrl::~ListCtrl()
{
}

bool ListCtrl::Create(DWORD dwStyle, CRect & Rect, CWnd * ParentWnd, UINT nId)
{
	m_ParentWnd = ParentWnd;

	if(CListCtrl::Create(dwStyle | LVS_REPORT | LVS_NOSORTHEADER | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, Rect, ParentWnd, nId))
	{
		ModifyStyleEx(0, WS_EX_CLIENTEDGE);
		return true;
	}

	return false;
}

void ListCtrl::AddColumn(const char * Text, DWORD Width, DWORD Id)
{
	Column * c;

	c = MAlloc(Column, 1);

	c->m_Text = Text;
	c->m_Width = Width;
	c->m_Id = Id;
	c->m_Enabled = true;

	m_Columns.PushBack(c);

	CListCtrl::InsertColumn(m_Columns.GetSize() - 1, Text, LVCFMT_LEFT, Width, Id);
}

void ListCtrl::OnDestroy()
{
	CWnd::OnDestroy();

	while(m_Columns.GetSize())
	{
		MFree(m_Columns[0]);
		m_Columns.Delete(0);
	}

	while(m_Items.GetSize())
	{
		MFree(m_Items[0]);
		m_Items.Delete(0);
	}
}

void ListCtrl::OnSize(UINT nType, int cx, int cy)
{
	DWORD i, j;
	CRect iRect;

	CWnd::OnSize(nType, cx, cy);

	for(i = 0; i < m_Items.GetSize(); i++)
	{
		for(j = 0; j < m_Items[i]->m_Data.GetSize(); j++)
		{
			if(m_Items[i]->m_Data[j]->m_Control)
			{
				m_Items[i]->m_Data[j]->m_Control->Invalidate(FALSE);
			}
		}
	}
}

ListItem ListCtrl::InsertItem(int Index)
{
	ListItem i;
	CRect cRect;
	int Pos;

	GetClientRect(&cRect);

	i = MAlloc(ListCtrl::Item, 1);

	m_Items.Insert(Index, i);

	Pos = CListCtrl::InsertItem(CListCtrl::GetItemCount(), "");
	CListCtrl::SetItemData(Pos, (DWORD_PTR)i);

	return i;
}

ListItem ListCtrl::AddItem(void)
{
	return InsertItem(m_Items.GetSize());
}

//void ListCtrl::ShowColumn(DWORD Id, bool Visible)
//{
//	DWORD i;
//
//	for(i = 0; i < m_Columns.GetSize(); i++)
//	{
//		if(m_Columns[i]->m_Id == Id)
//		{
//			m_Columns[i]->m_Enabled = Visible;
//			return;
//		}
//	}
//	
//	ASSERT(FALSE);
//}

void ListCtrl::DeleteItem(ListItem Item)
{
	DWORD i, j;

	for(i = 0; i < m_Items.GetSize(); i++)
	{
		if(m_Items[i] == Item)
		{
			for(j = 0; j < Item->m_Data.GetSize(); j++)
			{
				if(Item->m_Data[j]->m_Control)
				{
					Item->m_Data[j]->m_Control->ShowWindow(SW_HIDE);
				}
			}

			CListCtrl::DeleteItem(ListItemToIndex(Item));

			MFree(Item);
			m_Items.Delete(i);

			return;
		}
	}

	ASSERT(FALSE);
}

void ListCtrl::SetItemText(ListItem Item, const char * Text, DWORD Id)
{
	ItemData * d;
	int Index;
	int Col;

	d = IdToItemData(Item, Id, true);

	if(d->m_Text != Text)
	{
		d->m_Text = Text;

		Index = ListItemToIndex(Item);
		Col = IdToCol(Id);
		if(Col != -1)
		{
			CListCtrl::SetItemText(Index, Col, Text);
		}
	}
}

void ListCtrl::SetItemControl(ListItem Item, CWnd * Control, DWORD Id)
{
	ItemData * d;

	ASSERT(Control != NULL);

	d = IdToItemData(Item, Id, true);

	d->m_Control = Control;

	Invalidate(FALSE);
}

void ListCtrl::SetItemImage(ListItem Item, DWORD Image)
{
	LVITEM i;

	ZeroMemory(&i, sizeof(i));

	i.mask = LVIF_IMAGE;
	i.iImage = Image;
	i.iItem = ListItemToIndex(Item);

	Item->m_Image = Image;
	if(!CListCtrl::SetItem(&i))
	{
//		__asm int 3;
	}
}

int ListCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

//	if(!m_Images.Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 25))
//	{
//		__asm int 3;
//	}

//	CListCtrl::SetImageList(&m_Images, LVSIL_SMALL);

	return 0;
}

ListCtrl::ItemData * ListCtrl::IdToItemData(ListItem Item, DWORD Id, bool CreateIfNotExist)
{
	ItemData * d;
	DWORD i;

	ASSERT(Item != NULL);

	for(i = 0; i < Item->m_Data.GetSize(); i++)
	{
		if(Item->m_Data[i]->m_Id == Id)
		{
			return Item->m_Data[i];
		}
	}

	if(CreateIfNotExist)
	{
		d = MAlloc(ItemData, 1);

		d->m_Id = Id;
		d->m_Control = NULL;
		d->m_ControlVisible = false;

		Item->m_Data.PushBack(d);

		return d;
	}

	return NULL;
}

int ListCtrl::ListItemToIndex(ListItem Item)
{
	int i;

	for(i = 0; i < CListCtrl::GetItemCount(); i++)
	{
		if(CListCtrl::GetItemData(i) == (DWORD_PTR)Item)
		{
			return i;
		}
	}

	return -1;
}

//void ListCtrl::AddImage(DWORD ResourceId)
//{
//	m_Images.Add(AfxGetApp()->LoadIcon(ResourceId));
//}

void ListCtrl::OnPaint()
{
	CListCtrl::OnPaint();

	UpdateControls(false);
}

void ListCtrl::OnHdnItemchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);

	UpdateControls(true);

	Invalidate(FALSE);

	*pResult = 0;
}

void ListCtrl::UpdateControls(bool Invalidate)
{
	DWORD i, j;
	int Index;
	CRect iRect;
	int Col;

	for(i = 0; i < m_Items.GetSize(); i++)
	{
		for(j = 0; j < m_Items[i]->m_Data.GetSize(); j++)
		{
			if(m_Items[i]->m_Data[j]->m_Control)
			{
				Index = ListItemToIndex(m_Items[i]);

				Col = IdToCol(m_Items[i]->m_Data[j]->m_Id);

				if(Col == -1)
				{
					m_Items[i]->m_Data[j]->m_Control->ShowWindow(SW_HIDE);
				}else{
					CListCtrl::GetSubItemRect(Index, m_Items[i]->m_Data[j]->m_Id, LVIR_LABEL, iRect);

					if(m_Items[i]->m_Data[j]->m_ControlRect != iRect)
					{
						if(iRect.top < 16)
						{
							m_Items[i]->m_Data[j]->m_Control->ShowWindow(SW_HIDE);
						}else{
							m_Items[i]->m_Data[j]->m_Control->ShowWindow(SW_SHOW);
						}

						m_Items[i]->m_Data[j]->m_Control->MoveWindow(iRect);

						if(Invalidate)
						{
							m_Items[i]->m_Data[j]->m_Control->Invalidate(FALSE);
						}
					}
				}
			}
		}
	}
}

POSITION ListCtrl::GetFirstSelectedItemPosition(void)
{
	return CListCtrl::GetFirstSelectedItemPosition();
}

ListItem ListCtrl::GetNextSelectedItem(POSITION & Pos)
{
	int Index;

	Index = CListCtrl::GetNextSelectedItem(Pos);

	return (ListItem)CListCtrl::GetItemData(Index);
}

void ListCtrl::SetItemData(ListItem Item, DWORD_PTR Data)
{
	if(Item)
	{
		Item->m_CustomData = Data;
	}
}

DWORD_PTR ListCtrl::GetItemData(ListItem Item)
{
	if(Item)
	{
		return Item->m_CustomData;
	}

	return 0;
}

void ListCtrl::OnSetFocus(CWnd* pOldWnd)
{
	CListCtrl::OnSetFocus(pOldWnd);

	UpdateControls(true);
}

void ListCtrl::SetActiveFlags(DWORD f)
{
	DWORD i;

	f |= 0x0001;

	for(i = 0; i < m_Columns.GetSize(); i++)
	{
		if(m_Columns[i]->m_Enabled)
		{
			CListCtrl::DeleteColumn(0);
		}
	}

	for(i = 0; i < m_Columns.GetSize(); i++)
	{
		if(f & (1 << i))
		{
			m_Columns[i]->m_Enabled = true;
		}else{
			m_Columns[i]->m_Enabled = false;
		}
	}

	for(i = 0; i < m_Columns.GetSize(); i++)
	{
		if(m_Columns[i]->m_Enabled)
		{
			CListCtrl::InsertColumn(i, m_Columns[i]->m_Text, LVCFMT_LEFT, m_Columns[i]->m_Width, m_Columns[i]->m_Id);
		}
	}

	Invalidate(false);
	UpdateListData();
}

int ListCtrl::IdToCol(DWORD Id)
{
	DWORD i;
	int Col;

	Col = 0;
	for(i = 0; i < m_Columns.GetSize(); i++)
	{
		if(m_Columns[i]->m_Id == Id)
		{
			if(m_Columns[i]->m_Enabled)
			{
				return Col;
			}else{
				return -1;
			}
		}

		if(m_Columns[i]->m_Enabled)
		{
			Col++;
		}
	}

	return -1;
}

void ListCtrl::UpdateListData(void)
{
	DWORD i, j, k;
	CString Text;

	for(i = 0; i < m_Items.GetSize(); i++)
	{
		for(j = 0; j < m_Columns.GetSize(); j++)
		{
			if(m_Columns[j]->m_Enabled)
			{
				for(k = 0; k < m_Items[i]->m_Data.GetSize(); k++)
				{
					if(m_Items[i]->m_Data[k]->m_Id == m_Columns[j]->m_Id)
					{
						Text = m_Items[i]->m_Data[k]->m_Text;
						m_Items[i]->m_Data[k]->m_Text = "<!>";
						SetItemText(m_Items[i], Text, m_Columns[j]->m_Id);
						break;
					}
				}
				if(k == m_Items[i]->m_Data.GetSize())
				{
					SetItemText(m_Items[i], "", m_Columns[j]->m_Id);
				}
			}
		}
	}
}
