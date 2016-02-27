#include <stdio.h>
#include "RSGDIFont.h"
#include "Realspace.h"

#define WIDTHBYTES(i)       ((i+31)/32*4)
#define PALETTE_SIZE        2

RSGDIFont::RSGDIFont()
{
	m_hFont=NULL;
	m_hMemDC=NULL;
	m_hBitmap=NULL;
	m_iBmpWidth=0;
	m_iBmpHeight=0;
	m_dwColor=0xffffff;
	m_pBmpBits=NULL;
}

RSGDIFont::~RSGDIFont()
{
	Destroy();
}

void RSGDIFont::Destroy()
{
    if (m_hMemDC != NULL)
    {
        DeleteDC(m_hMemDC);
        m_hMemDC = NULL;
    }
    if (m_hBitmap != NULL)
    {
        DeleteObject(m_hBitmap);
        m_hBitmap = NULL;
    }
    if (m_hFont != NULL)
    {
        DeleteObject(m_hFont);
        m_hFont = NULL;
    }
}

void RSGDIFont::SetClipRect(RSVIEWPORT *pClipRect)
{ 
	m_ClipRect.left=pClipRect->x1;
	m_ClipRect.right=pClipRect->x2;
	m_ClipRect.top=pClipRect->y1;
	m_ClipRect.bottom=pClipRect->y2;
}

bool RSGDIFont::Create(const char *fontname,int nSize,bool bBold,bool bItalic,RSPIXELFORMAT pf,RSVIEWPORT *ClipRect)
{
	m_PixelFormat=pf;
	SetClipRect(ClipRect);

    HDC				hdc;
    LPBITMAPINFO	pbmi;

    m_hFont = CreateFont(nSize,
		0, 0, 0, bBold?FW_BOLD:FW_NORMAL, bItalic?TRUE:FALSE, FALSE, FALSE,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		VARIABLE_PITCH,
		fontname);
    if (m_hFont == NULL)
        return FALSE;

    // Create a memory DC for rendering our text into
    hdc = GetDC(HWND_DESKTOP);
    m_hMemDC = CreateCompatibleDC(hdc);
    ReleaseDC(NULL, hdc);
    if (m_hMemDC == NULL)
    {
        DeleteObject(m_hFont);
        return FALSE;
    }

	SetMapMode(m_hMemDC,MM_TEXT);
    // Select font, and get text dimensions
    SelectObject(m_hMemDC, m_hFont);
    m_iBmpWidth = ClipRect->x2-ClipRect->x1 + 2;
    m_iBmpHeight = ClipRect->y2-ClipRect->y1 + 2;

    // Create a dib section for containing the bits
    pbmi = (LPBITMAPINFO) LocalAlloc(LPTR, sizeof(BITMAPINFO) + PALETTE_SIZE * sizeof(RGBQUAD));
    if (pbmi == NULL)
    {
        DeleteObject(m_hFont);
        DeleteDC(m_hMemDC);
        return false;
    }

    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pbmi->bmiHeader.biWidth = m_iBmpWidth;
    pbmi->bmiHeader.biHeight = -1 * m_iBmpHeight;  // negative height = top-down
    pbmi->bmiHeader.biPlanes = 1;
    pbmi->bmiHeader.biBitCount = 8;  // 8bpp makes it easy to get data

    pbmi->bmiHeader.biCompression = BI_RGB;
    pbmi->bmiHeader.biXPelsPerMeter = 0;
    pbmi->bmiHeader.biYPelsPerMeter = 0;
    pbmi->bmiHeader.biClrUsed = PALETTE_SIZE;
    pbmi->bmiHeader.biClrImportant = PALETTE_SIZE;

    pbmi->bmiHeader.biSizeImage = WIDTHBYTES(m_iBmpWidth * 8) * m_iBmpHeight;

    // Just a plain monochrome palette
    pbmi->bmiColors[0].rgbRed = 0;
    pbmi->bmiColors[0].rgbGreen = 0;
    pbmi->bmiColors[0].rgbBlue = 0;
    pbmi->bmiColors[1].rgbRed = 255;
    pbmi->bmiColors[1].rgbGreen = 255;
    pbmi->bmiColors[1].rgbBlue = 255;

    // Create a DIB section that we can use to read the font bits out of
    m_hBitmap = CreateDIBSection(m_hMemDC,
                                 pbmi,
                                 DIB_RGB_COLORS,
                                 (void **) &m_pBmpBits,
                                 NULL,
                                 0);
    LocalFree(pbmi);
    if (m_hBitmap == NULL)
    {
        DeleteObject(m_hFont);
        DeleteDC(m_hMemDC);
        return false;
    }

    // Set up our memory DC with the font and bitmap
    SelectObject(m_hMemDC, m_hBitmap);
    SetBkColor(m_hMemDC, RGB(0, 0, 0));
    SetTextColor(m_hMemDC, RGB(255, 255, 255));
    return true;
}

void RSGDIFont::DrawText(void *pBuffer,int lPitch,int x,int y,LPSTR text,...)
{
    char    buff[2048];
    va_list	vlist;
    LPSTR   lpbits,lpsrc;
	int		bytes_pixel,i,j;

	RECT	rtori,rtfinal;

    // Get message to display
    va_start(vlist, text);
    vsprintf(buff, text, vlist);
    va_end(vlist);

	SIZE	size;
	GetTextExtentPoint32(m_hMemDC, buff, strlen(buff), &size);
	size.cx++;size.cy++;

	rtori.left=x;
	rtori.top=y;
	rtori.right=x+size.cx;
	rtori.bottom=y+size.cy;

	if(IntersectRect(&rtfinal,&rtori,&m_ClipRect))
	{
		int		xoffset,yoffset,width,height;

		height=rtfinal.bottom-rtfinal.top;
		width=rtfinal.right-rtfinal.left;
		xoffset=rtfinal.left-x;
		yoffset=rtfinal.top-y;

		// Output text to our memory DC (the bits end up in our DIB section)
		PatBlt(m_hMemDC, 0, 0, size.cx, size.cy, BLACKNESS);
		TextOut(m_hMemDC, 1, 1, buff, lstrlen(buff));

		// 16bit version only. 필요하면 요기다 추가할수도 있겠져?
		
		bytes_pixel=RSPixelFormat == RS_PIXELFORMAT_X888 ? 4 : 2;

		if(bytes_pixel==4)
		{
			// Get pointer to place on screen we want to copy the text to
			lpbits = &(((LPSTR) pBuffer)[rtfinal.top * lPitch + x * bytes_pixel]);
			lpsrc = m_pBmpBits;
			lpsrc += WIDTHBYTES(m_iBmpWidth * 8) * yoffset;

			for (i = 0; i < height; i++)
			{
				for (j = xoffset; j < xoffset+width; j++)
				{
					if (lpsrc[j])
						((DWORD *) lpbits)[j] = m_dwColor;
				}
				lpbits += lPitch;
				lpsrc += WIDTHBYTES(m_iBmpWidth * 8);
			}
		}
		else
		{
			WORD color=RGB322RGB16(m_dwColor);

			// Get pointer to place on screen we want to copy the text to
			lpbits = &(((LPSTR) pBuffer)[rtfinal.top * lPitch + x * bytes_pixel]);
			lpsrc = m_pBmpBits;
			lpsrc += WIDTHBYTES(m_iBmpWidth * 8) * yoffset;

			for (i = 0; i < height; i++)
			{
				for (j = xoffset; j < xoffset+width; j++)
				{
					if (lpsrc[j])
						((WORD *) lpbits)[j] = color;
/*					else
					{
		                ((WORD *) lpbits)[j] = 0x0000;
					}
*/
				}
				lpbits += lPitch;
				lpsrc += WIDTHBYTES(m_iBmpWidth * 8);
			}
		}
	}
}

void RSGDIFont::GetTextExtentPoint(const char *string, SIZE *lpsize)
{
	GetTextExtentPoint32(m_hMemDC, string, strlen(string), lpsize);
	//lpsize->cx++;lpsize->cy++;
}
