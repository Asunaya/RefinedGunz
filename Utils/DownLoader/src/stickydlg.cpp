#include "stdafx.h"
#include "stickydlg.h"

StickyDlg::StickyDlg(CWnd* pParent /*=NULL*/)
: CDialog(StickyDlg::IDD, pParent)
{
	m_RootWindow = NULL;
}

StickyDlg::~StickyDlg()
{
	DeleteWindows();
}

void StickyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


IMPLEMENT_DYNAMIC(StickyDlg, CDialog)
BEGIN_MESSAGE_MAP(StickyDlg, CDialog)
	ON_WM_NCLBUTTONDOWN()
	ON_WM_MOVING()
	ON_WM_SIZING()
	ON_WM_PAINT()
END_MESSAGE_MAP()

BOOL StickyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void StickyDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
	}
	else
	{
		CDialog::OnPaint();
	}
}

BOOL CALLBACK StickyDlg::EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	if(!::IsWindowVisible(hWnd)) return TRUE;
	if(hWnd == ((StickyDlg *)lParam)->m_hWnd) return TRUE;

	((StickyDlg *)lParam)->AddWindow(hWnd);

	return TRUE;
}

void StickyDlg::AddWindow(HWND hWnd)
{
	Window * w;

	w = new Window;
	w->m_hWnd = hWnd;
	w->m_Next = m_RootWindow;

	m_RootWindow = w;
}

void StickyDlg::DeleteWindows(void)
{
	Window * w;

	while(m_RootWindow)
	{
		w = m_RootWindow;
		m_RootWindow = m_RootWindow->m_Next;

		delete w;
	}
}

void StickyDlg::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	CRect wRect;

	DeleteWindows();
	EnumWindows(EnumWindowsProc, (LPARAM)this);
	AddWindow(::GetDesktopWindow());

	GetWindowRect(&wRect);
	GetCursorPos(&m_MouseOffset);

	m_MouseOffset -= wRect.TopLeft();

	CDialog::OnNcLButtonDown(nHitTest, point);
}

void StickyDlg::OnMoving(UINT fwSide, LPRECT pRect)
{
	CDialog::OnMoving(fwSide, pRect);

	SnapWindow(false, pRect);
}

void StickyDlg::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialog::OnSizing(fwSide, pRect);

	SnapWindow(true, pRect);
}

void StickyDlg::SnapWindow(bool Sizing, LPRECT pRect)
{
	Window * w;
	CRect wRect;	// Window Rect
	CRect iRect;	// Inflated Rect
	CRect dRect;	// Dummy Rect
	CPoint MouseOffset;
	
	if(!Sizing)
	{
		GetCursorPos(&MouseOffset);
		MouseOffset -= CRect(pRect).TopLeft();

		OffsetRect(pRect, MouseOffset.x - m_MouseOffset.x, MouseOffset.y - m_MouseOffset.y);
	}

	for(w = m_RootWindow; w != NULL; w = w->m_Next)
	{
		::GetWindowRect(w->m_hWnd, &wRect);

		iRect = *pRect;
		iRect.InflateRect(STICKYNESS, STICKYNESS);
		if(dRect.IntersectRect(&iRect, &wRect))
		{
			// Left Side
			if(abs(pRect->left - wRect.left) <= STICKYNESS)
			{
				if(Sizing)
				{
					pRect->left += -(pRect->left - wRect.left);
				}else{
					OffsetRect(pRect, -(pRect->left - wRect.left), 0);
				}
			}
			if(abs(pRect->left - wRect.right) <= STICKYNESS)
			{
				if(Sizing)
				{
					pRect->left += -(pRect->left - wRect.right);
				}else{
					OffsetRect(pRect, -(pRect->left - wRect.right), 0);
				}
			}

			// Right Side
			if(abs(pRect->right - wRect.left) <= STICKYNESS)
			{
				if(Sizing)
				{
					pRect->right += -(pRect->right - wRect.left);
				}else{
					OffsetRect(pRect, -(pRect->right - wRect.left), 0);
				}
			}
			if(abs(pRect->right - wRect.right) <= STICKYNESS)
			{
				if(Sizing)
				{
					pRect->right += -(pRect->right - wRect.right);
				}else{
					OffsetRect(pRect, -(pRect->right - wRect.right), 0);
				}
			}

			// Top Side
			if(abs(pRect->top - wRect.top) <= STICKYNESS)
			{
				if(Sizing)
				{
					pRect->top += -(pRect->top - wRect.top);
				}else{
					OffsetRect(pRect, 0, -(pRect->top - wRect.top));
				}
			}
			if(abs(pRect->top - wRect.bottom) <= STICKYNESS)
			{
				if(Sizing)
				{
					pRect->top += -(pRect->top - wRect.bottom);
				}else{
					OffsetRect(pRect, 0, -(pRect->top - wRect.bottom));
				}
			}

			// Bottom Side
			if(abs(pRect->bottom - wRect.top) <= STICKYNESS)
			{
				if(Sizing)
				{
					pRect->bottom += -(pRect->bottom - wRect.top);
				}else{
					OffsetRect(pRect, 0, -(pRect->bottom - wRect.top));
				}
			}
			if(abs(pRect->bottom - wRect.bottom) <= STICKYNESS)
			{
				if(Sizing)
				{
					pRect->bottom += -(pRect->bottom - wRect.bottom);
				}else{
					OffsetRect(pRect, 0, -(pRect->bottom - wRect.bottom));
				}
			}
		}
	}
}
