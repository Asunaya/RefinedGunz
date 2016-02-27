#include "stdafx.h"
#include "MBitmap.h"
#include "MDrawContext.h"


// MBitmap Implementation
/////////////////////////
MBitmap::MBitmap(void)
{
#ifdef _DEBUG
	m_nTypeID = MINT_BASE_CLASS_TYPE;
#endif
	m_DrawMode = MBM_Normal;
}

MBitmap::~MBitmap(void)
{
	Destroy();
}

bool MBitmap::Create(const char* szName)
{
	_ASSERT(strlen(szName)<MBITMAP_NAME_LENGTH);
	strcpy(m_szName, szName);

	return true;
}

void MBitmap::Destroy(void)
{
}

MAniBitmap::MAniBitmap(void)
{
	m_nCurFrame = 0;
}

MAniBitmap::~MAniBitmap(void)
{
	Destroy();
}

bool MAniBitmap::Create(const char* szName)
{
	_ASSERT(strlen(szName)<MBITMAP_NAME_LENGTH);
	strcpy(m_szName, szName);

	return true;
}

void MAniBitmap::Destroy(void)
{
	m_Bitmaps.DeleteRecordAll();
}

void MAniBitmap::Add(MBitmap* pBitmap)
{
	m_Bitmaps.Add(pBitmap);
}

MBitmap* MAniBitmap::Get(int nFrame)
{
	if(nFrame<0 || nFrame>=GetFrameCount()) return NULL;
	return m_Bitmaps.Get(nFrame);
}

MBitmap* MAniBitmap::Get(void)
{
	return Get(m_nCurFrame);
}

int MAniBitmap::GetFrameCount(void)
{
	return m_Bitmaps.GetCount();
}

int MAniBitmap::GetCurFrame(void)
{
	return m_nCurFrame;
}

bool MAniBitmap::MoveNext(void)
{
	if(m_nCurFrame+1>=GetFrameCount()) return false;
	m_nCurFrame++;
	return true;
}

bool MAniBitmap::MovePrevious(void)
{
	if(m_nCurFrame-1<0) return false;
	m_nCurFrame--;
	return true;
}

void MAniBitmap::MoveFirst(void)
{
	m_nCurFrame = 0;
}

void MAniBitmap::MoveLast(void)
{
	m_nCurFrame = GetFrameCount();
}

bool MAniBitmap::Move(int nFrame)
{
	if(nFrame<0 || nFrame>=GetFrameCount()) return false;
	m_nCurFrame = nFrame;
	return true;
}

int MAniBitmap::GetDelay(void)
{
	return m_nDelay;
}

MPartialBitmap::MPartialBitmap()
{
	m_pSource = NULL;
}

MPartialBitmap::MPartialBitmap(MBitmap *pBitmap, MRECT rt)
{
	m_pSource = pBitmap;
	m_Rect = rt;
}
