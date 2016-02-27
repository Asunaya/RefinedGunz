#ifndef MBITMAP_H
#define MBITMAP_H

#include "CMList.h"
#include "MTypes.h"

#define MBM_Normal 0
#define MBM_FlipLR 1
#define MBM_FlipUD 1<<1
#define MBM_RotL90 1<<2
#define MBM_RotR90 1<<3

////////////////////////////////////////////////////
// Bitmap abstract class
//		시스템에 맞는 class로 override해서 사용한다.
#define MBITMAP_NAME_LENGTH		128
class MBitmap{
public:
#ifdef _DEBUG
	int		m_nTypeID;	// 같은 레벨의 클래스(MFont, MDrawContex)를 사용하기 위한 ID
#endif

public:
	char	m_szName[MBITMAP_NAME_LENGTH];
	DWORD	m_DrawMode;
public:
	MBitmap(void);
	virtual ~MBitmap(void);

	void CreatePartial(MBitmap *pBitmap,MRECT rt,const char *szName);		// partial bitmap 으로 생성한다

	virtual bool Create(const char* szName);
	virtual void Destroy(void);

	virtual void SetDrawMode(DWORD md) { m_DrawMode = md; }
	virtual DWORD GetDrawMode() { return m_DrawMode; }

	virtual int GetX(void) { return 0; }
	virtual int GetY(void) { return 0; }
	virtual int GetWidth(void) = 0;
	virtual int GetHeight(void) = 0;

	virtual MBitmap *GetSourceBitmap() { return this; }
};

class MPartialBitmap : public MBitmap {
	MBitmap	*m_pSource;
	MRECT	m_Rect;
public:
	MPartialBitmap();
	MPartialBitmap(MBitmap *pBitmap, MRECT rt);

	virtual int GetX(void) { return m_Rect.x; }
	virtual int GetY(void) { return m_Rect.y; }
	virtual int GetWidth(void) { return m_Rect.w; }
	virtual int GetHeight(void) { return m_Rect.h; }

	virtual MBitmap *GetSourceBitmap() { return m_pSource; }
};

class MAniBitmap{
protected:
#ifdef _DEBUG
	friend class MDrawContext;
	int		m_nTypeID;		// 같은 레벨의 클래스(MFont, MDrawContex)를 사용하기 위한 ID
#endif
public:
	char	m_szName[MBITMAP_NAME_LENGTH];
private:
	CMLinkedList<MBitmap>	m_Bitmaps;
protected:
	int		m_nCurFrame;
	int		m_nDelay;
public:
	MAniBitmap(void);
	virtual ~MAniBitmap(void);
	bool Create(const char* szName);
	void Destroy(void);

	void Add(MBitmap* pBitmap);
	MBitmap* Get(int nFrame);
	MBitmap* Get(void);

	int GetFrameCount(void);
	int GetCurFrame(void);

	bool MoveNext(void);
	bool MovePrevious(void);
	void MoveFirst(void);
	void MoveLast(void);
	bool Move(int nFrame);

	int GetDelay(void);
	void SetDelay(int nDelay) { m_nDelay = nDelay; }
};

#endif