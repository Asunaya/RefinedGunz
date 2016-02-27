#include "stdafx.h"

#include "ZApplication.h"
#include "ZToolTip.h"
#include "Mint4R2.h"

//#include "MTextArea.h"

#define ZTOOLTIP_WIDTH_GAP 10
#define ZTOOLTIP_HEIGHT_GAP 10

#define ZTOOLTIP_MAX_W 350
#define ZTOOLTIP_LINE_GAP 3

ZToolTip::ZToolTip(const char* szName, MWidget* pParent)
: MToolTip(szName, pParent)
{
//	m_pBitmap1 = NULL;
//	m_pBitmap2 = NULL;

//	m_pBitmap1 = MBitmapManager::Get("arrow butten_left.tga");
	m_pBitmap1 = MBitmapManager::Get("tooltip_edge01.png");
	m_pBitmap2 = MBitmapManager::Get("tooltip_edge02.png");

	SetBounds();

//	m_pTextArea = new MTextArea;
//	m_pTextArea->MTextArea( ZTOOLTIP_MAX_W );
}

ZToolTip::~ZToolTip(void)
{

}

/*
bold 빼고 가는 글씨로~

- 버튼 위에서 움직이는 경우 사라지도록? - 윈도우 툴팁처럼..( 혹은 따라서 움직이지는 말도록 )
- 상하좌우 여백은 ( 10,10 ) 
- 투명하지 않게
*/

bool IsToolTipEnable();

void ZToolTip::OnDraw(MDrawContext* pDC)
{
/*
	if(IsToolTipEnable()==false) {
		return;
	}
*/
	MRECT r = GetClientRect();

//	pDC->SetColor(MCOLOR(DEFCOLOR_MTOOLTIP_PLANE));
//	pDC->FillRectangle(r);
//	pDC->SetColor(MCOLOR(DEFCOLOR_MTOOLTIP_OUTLINE));
//	pDC->Rectangle(r);

	// 최소사이즈 32 x 32 -> 16 2개씩은 찍을 수 있도록~
	if(m_pBitmap1&&m_pBitmap2) {

		//자신의 영역을 둘러싸는 이미지를 그려준다..
		// 9곳 회전해서 그리기..
		// 1 2 3 
		// 4 5 6
		// 7 8 9

		m_pBitmap1->SetDrawMode(MBM_Normal);
		m_pBitmap2->SetDrawMode(MBM_Normal);

		pDC->SetBitmap( m_pBitmap1 );
		pDC->Draw(r.x, r.y, 16, 16);

		pDC->SetBitmap( m_pBitmap2 );
		pDC->Draw(r.x+16, r.y, r.w-32,16);

		m_pBitmap1->SetDrawMode(MBM_FlipLR);

		pDC->SetBitmap( m_pBitmap1 );
		pDC->Draw(r.x+r.w-16, r.y, 16, 16);

		//그려야할사이즈가있다면~ 중간단계
		if(r.h > 32) {

			m_pBitmap2->SetDrawMode(MBM_RotL90);
			pDC->SetBitmap( m_pBitmap2 );
			pDC->Draw(r.x, r.y+16, 16, r.h-32);

			//중간색채우기
			pDC->SetColor(MCOLOR(0xffD9D9D9));//임시
			pDC->FillRectangle(MRECT(r.x+16,r.y+16,r.w-32,r.h-32));

			m_pBitmap2->SetDrawMode(MBM_RotR90);
			pDC->SetBitmap( m_pBitmap2 );
			pDC->Draw(r.x+r.w-16, r.y+16, 16, r.h-32);
		}

		// 아래부분~

		m_pBitmap1->SetDrawMode(MBM_FlipUD);
		pDC->SetBitmap( m_pBitmap1 );
		pDC->Draw(r.x, r.y+r.h-16, 16, 16);

		m_pBitmap2->SetDrawMode(MBM_FlipUD);
		pDC->SetBitmap( m_pBitmap2 );
		pDC->Draw(r.x+16, r.y+r.h-16, r.w-32,16);

		m_pBitmap1->SetDrawMode(MBM_FlipUD|MBM_FlipLR);
		pDC->SetBitmap( m_pBitmap1 );
		pDC->Draw(r.x+r.w-16, r.y+r.h-16, 16, 16);

		m_pBitmap1->SetDrawMode(MBM_Normal);
		m_pBitmap2->SetDrawMode(MBM_Normal);
	}

	char* szName = NULL;

	if(m_bUseParentName==true) 
		szName = GetParent()->m_szName;
	else 
		szName = m_szName;

	MRECT text_r = MRECT(r.x+10,r.y+10,r.w-10,r.h-10);

//	pDC->SetColor(MCOLOR(DEFCOLOR_MTOOLTIP_TEXT));//임시
	pDC->SetColor(MCOLOR(0xff000000));//임시
//	pDC->TextWithHighlight(text_r, szName, (MAM_HCENTER|MAM_VCENTER));
	pDC->TextMultiLine(text_r, szName,ZTOOLTIP_LINE_GAP);	
}

int GetLineCount(char* str,int& max) {

	if(!str || str[0]==NULL)
		return 0;

	int line = 0;

	max = 0;

	int cnt = (int)strlen(str);
	int back = 0;

	for(int i=0;i<cnt;i++) {
		if(str[i]=='\n') {
			line++;

			max = max((i - back),max); // 라인폭..
			back = i;
		}
	}
	return line;
}

void ZToolTip::SetBounds(void)
{
	MFont* pFont = GetFont();

	char szName[MWIDGET_NAME_LENGTH];

	RemoveAnd(szName, m_bUseParentName==true?GetParent()->m_szName:m_szName);

	int nWidth = pFont->GetWidthWithoutAmpersand(szName);
	int nHeight = pFont->GetHeight();
	int x, y;

	MRECT pr = GetParent()->GetClientRect();
	MRECT spr = MClientToScreen(GetParent(), pr);

	if(spr.x+(nWidth+ZTOOLTIP_WIDTH_GAP/2)<=MGetWorkspaceWidth())
		x = pr.x+ZTOOLTIP_WIDTH_GAP/2+1;
	else{
		MPOINT p = MScreenToClient(GetParent(), MPOINT(MGetWorkspaceWidth()-(nWidth+ZTOOLTIP_WIDTH_GAP/2), 0));
		x = p.x;
	}

	y = pr.y-(nHeight+ZTOOLTIP_HEIGHT_GAP);
	MPOINT p = MClientToScreen(GetParent(), MPOINT(0, y));

	if(p.y<0) {
		y = p.y+pr.h+(nHeight+ZTOOLTIP_HEIGHT_GAP);
		if(y>MGetWorkspaceHeight()) y = 0;
		p = MScreenToClient(GetParent(), MPOINT(0, y));
		y = p.y;
	}
	
	// 최소사이즈 16 size bitmap 일경우.. 32x32 규정?
	int w = max(nWidth+ZTOOLTIP_WIDTH_GAP*2,32);
	int h = max(nHeight+ZTOOLTIP_HEIGHT_GAP*2,32);

	if(w) {

//		int line = (w / (ZTOOLTIP_MAX_W-20))+1;
		int _max=0;
		int line = GetLineCount( szName , _max );
		int	_max_w = MAX_TOOLTIP_LINE_STRING*(pFont->GetWidth("b"));
		
		if(line) {
			w = _max_w;
			h = h + ((nHeight + ZTOOLTIP_LINE_GAP) * line);
		}
//		else w = min(w,_max_w);
	}

	MWidget::SetBounds(MRECT(x-ZTOOLTIP_WIDTH_GAP, y-ZTOOLTIP_HEIGHT_GAP,w,h));
}
