#ifndef MPICTURE_H
#define MPICTURE_H

#include "MWidget.h"

/// Picture
class MPicture : public MWidget{
protected:
	MBitmap* m_pBitmap;
	int		m_iStretch;
	int		m_iAnimType;			// 0:좌->우, 1:우->좌
	float	m_fAnimTime;			// 애니메이션 되는 시간
	DWORD	m_dwCurrentTime;
	bool	m_bAnim;
	DWORD	m_DrawMode;

	MCOLOR	m_BitmapColor;
	MCOLOR	m_BitmapReseveColor;
	bool	m_bSwaped;

protected:
	/// Draw Handler
	virtual void OnDraw(MDrawContext* pDC);
	void OnAnimDraw(MDrawContext* pDC, int x, int y, int w, int h, int bx, int by, int bw, int bh );

public:
	MPicture(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);

	/// 비트맵 지정
	void SetBitmap(MBitmap* pBitmap);
	/// 비트맵 얻기
	MBitmap* GetBitmap(void);
	int		GetStretch() { return m_iStretch; }
	void SetStretch(int i) { m_iStretch = i; }

	void SetAnimation( int animType, float fAnimTime);

	void SetBitmapColor( MCOLOR color );	
	MCOLOR GetBitmapColor() const {
		return m_BitmapColor;
	}
	MCOLOR GetReservedBitmapColor() const{
		return m_BitmapReseveColor;	
	}

	void SetDrawMode( DWORD mode ) {
		m_DrawMode = mode;
	}
	DWORD GetDrawMode( ) const {
		return m_DrawMode;
	}

	bool IsAnim() const { return m_bAnim;}

#define MINT_PICTURE	"Picture"
	virtual const char* GetClassName(void){ return MINT_PICTURE; }
};


#endif