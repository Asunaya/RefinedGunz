#ifndef _ZCHARACTERSELECTVIEW_H
#define _ZCHARACTERSELECTVIEW_H

#include "ZMeshView.h"
#include "MUID.h"
#include "MMatchTransDataType.h"
#include "RCharCloth.h"
#include "ZShadow.h"

class ZInterfaceBackground;

// 여기서 쓰이는 index들은 캐릭터 슬롯의 index

struct ZSelectCharacterInfo
{
	MTD_CharInfo			m_CharInfo;
	MTD_AccountCharInfo		m_AccountCharInfo;
	bool					m_bLoaded;				// 해당 슬롯의 캐릭터 정보를 받아왔는지 여부
	bool					m_bRequested;			// 해당 슬롯의 캐릭터 정보를 요청했는지 여부
};


/// 캐릭터 선택할 때 필요한 것들을 모아 놓은 클래스. 
/// 캐릭터 정보도 이 클래스가 가지고 있고, 그리는 것도 이 클래스가 함
class ZCharacterSelectView
{
public:
	enum ZSelectViewState
	{
		ZCSVS_SELECT = 0,
		ZCSVS_CREATE
	};
private:
	static int				m_nNumOfCharacter;			// 가지고 있는 캐릭터 수
	static int				m_nSelectedCharacter;		// 선택한 캐릭터 인덱스
	static char				m_szLastChar[MATCHOBJECT_NAME_LENGTH];

	bool					m_bReserveSelectChar;
	int						m_nSelCharIndex;
	unsigned long int		m_nReservedSelectTime;
	RVisualMesh*			m_pVisualMesh;
	RMesh*					m_pMesh;		// 임시메쉬
	ZShadow*				m_pShadow;
	ZInterfaceBackground*	m_pBackground;
	ZSelectViewState		m_nState;
	float					m_fCRot;

	void DrawCharacterLight(rvector& vCharPos);
	static void ClearInterfaces();
public:
	static ZSelectCharacterInfo		m_CharInfo[MAX_CHAR_COUNT];		// 내 캐릭터 정보
	static void ClearCharInfo();
	static void OnReceivedAccountCharInfo(void* pCharListBlob);
	static void OnReceivedCharInfo(int nCharNum, void* pCharInfoBlob);
public:
	ZCharacterSelectView();
	~ZCharacterSelectView();
	void Draw(void);
	void SetBackground(ZInterfaceBackground* pBackground) { m_pBackground = pBackground; }
	bool SelectMyCharacter();
	void SelectChar(int nSelectIndex);
	void SetState(ZSelectViewState nState);
	void OnChangedCharCostume();
	void OnChangedCharInfo(int sex,int index);
	bool IsEmpty(int nIndex);
	void OnInvalidate();
	void OnRestore();
	void UpdateInterface(int nSelIndex);
	inline void CharacterLeft(float add_degree=1.0f);
	inline void CharacterRight(float add_degree=1.0f);
	static int GetNumOfCharacter() { return m_nNumOfCharacter; }
	static int GetSelectedCharacter() { return m_nSelectedCharacter; }
	static void SetSelectedCharacter(int nCharIndex) { m_nSelectedCharacter = nCharIndex; }
	static void SetSelectedCharacterFromLastChar();

	static void LoadLastChar();
	static void SetLastChar(char* szName);
	static const char* GetLastChar() { return m_szLastChar; }
};





// inline func /////////////////////////////////////////////////////////////////////////////
inline void ZCharacterSelectView::CharacterLeft(float add_degree)
{
	m_fCRot += add_degree;
}

inline void ZCharacterSelectView::CharacterRight(float add_degree)
{
	m_fCRot -= add_degree;
}





#endif