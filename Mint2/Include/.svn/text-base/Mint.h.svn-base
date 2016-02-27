#ifndef MINT_H
#define MINT_H

#ifdef WIN32
#include <windows.h>
#endif

#include <map>
using namespace std;

#include "MTypes.h"
#include "MEvent.h"


class MDrawContext;
class MWidget;
class MFont;
class MBitmap;
class MResourceMap;
class MIDLResource;
class MListener;
class MEvent; 

#define MINT_VERSION	2
#define	MVersion()	MINT_VERSION

/// first KeyValue, second ActionID
//typedef map<unsigned long int, int>	ACTIONKEYMAP;

/// Global Event Callback
typedef bool(MGLOBALEVENTCALLBACK)(MEvent* pEvent);

/// Mint Global Object Class
class Mint{
protected:
	static Mint*			m_pInstance;	///< Instance
	MWidget*				m_pMainFrame;	///< Main Frame Widget
	MDrawContext*			m_pDC;			///< DrawContext
	MGLOBALEVENTCALLBACK*	m_fnGlobalEventCallBack;	///< Global Event Callback function

	// Drag & Drop
//	int			m_nDragObjectID;
	char		m_szDragObjectString[256];
	char		m_szDragObjectItemString[256];
	MBitmap*	m_pDragObjectBitmap;
	MPOINT		m_GrabPoint;
	bool		m_bVisibleDragObject;
	MWidget*	m_pDropableObject;
	MWidget*	m_pDragSourceObject;

	// Workspace Size
	int			m_nWorkspaceWidth;
	int			m_nWorkspaceHeight;

	/// Action Map
//	ACTIONKEYMAP	m_ActionKeyMap;
//#define ACTIONKEYMAP_IDCOUNT	256
//	bool			m_ActionKeyPressedTable[ACTIONKEYMAP_IDCOUNT];

	// IME 관련
	void*	m_pCandidateList;		// Candidate List, LPCANDIDATELIST로 캐스팅해서 사용한다.
	int		m_nCandidateListSize;	// Candidate List Size
	MPOINT	m_CandidateListPos;		// Candidate List가 보여질 위치

	bool	m_bEnableIME;

public:
	DWORD	m_nCompositionAttributeSize;				// Composition 문자열의 속성 개수
	BYTE	m_nCompositionAttributes[MIMECOMPOSITIONSTRING_LENGTH];		// Composition 문자열의 속성
	//DWORD	m_dwCompositionClauses[MIMECOMPOSITIONSTRING_LENGTH];		// Composition 문자열의 속성에 따른 문자열 구분
	int		m_nCompositionCaretPosition;				// Composition 문자열에서의 캐럿 위치

protected:
	/// Candidate List 그리기, 위젯으로 생성하지 않고 직접 그린다
	virtual void DrawCandidateList(MDrawContext* pDC, MPOINT& p);

public:
	Mint(void);
	virtual ~Mint(void);

	/// Mint Initialize
	bool Initialize(int nWorkspaceWidth, int nWorkspaceHeight, MDrawContext* pDC, MFont* pDefaultFont);
	/// Mint Finalize
	void Finalize(void);

#ifdef WIN32
	/// Event Processing
	bool ProcessEvent(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
#endif

	/// Mint Run
	virtual void Run(void);
	/// Mint Draw
	virtual void Draw(void);

	/// Event Action Key
//	bool EventActionKey(unsigned long int nKey, bool bPressed);

	/// Update(Redraw)
	virtual void Update(void){}

	/// Get MainFrame Widget
	MWidget* GetMainFrame(void);

	/// Get DrawContext
	MDrawContext* GetDrawContext(void);

	/// Singleton Instance
	static Mint* GetInstance(void);

#ifdef WIN32
	// 윈도우 핸들 지정
	void SetHWND(HWND hWnd);
	HWND GetHWND(void);

	HIMC m_hImc;			// IME Handle, IME를 Enable/Disable하기 위한 보관용. IME관련 함수를 쓸때는 ImmGetContext()로 얻어서 사용한다. 만약, Disable되어 있으면, ImmGetContext()가 NULL을 돌려줄 것이다.
	//DWORD m_dwIMEConvMode;	// 현재의 IME 모드
#endif

	void EnableIME(bool bEnable);
	bool IsEnableIME(void);

	/// 핫키 추가
	int RegisterHotKey(unsigned long int nModifier, unsigned long int nVirtKey);
	/// 핫키 제거
	void UnregisterHotKey(int nID);

	/*
	/// 액션맵 추가
	bool RegisterActionKey(int nActionID, unsigned long int nKey);
	/// 액션맵 제거
	bool UnregisterActionKey(int nActionID);

	/// 키 번호로 키 이름 알아내기
	virtual const char* GetActionKeyName(unsigned long int nKey);
	/// ActionID로 키 번호 알아내기
	unsigned long int GetActionKey(int nActionID);

	/// 액션 버튼이 눌렸는가?
	bool IsActionKeyPressed(int nActionID);
	*/

	// Drag & Drop
	MWidget* SetDragObject(MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString);
	MWidget* GetDragObject(void);

	virtual MWidget* NewWidget(const char* szClass, const char* szName, MWidget* pParent, MListener* pListener);

	MWidget* FindWidgetDropAble(MPOINT& p);
	MWidget* FindWidget(MPOINT& p);
	MWidget* FindWidget(int x, int y);

	/// Mint가 동작될 화면의 가로 크기 얻기
	int GetWorkspaceWidth(void);
	/// Mint가 동작될 화면의 세로 크기 얻기
	int GetWorkspaceHeight(void);
	/// Mint가 동작될 화면의 크기 설정
	void SetWorkspaceSize(int w, int h);

	/// 시스템에 맞는 Bitmap Open Function을 제공해야 한다.
	virtual MBitmap* OpenBitmap(const char* szName) = 0;
	/// 시스템에 맞는 Font Open Function을 제공해야 한다.
	virtual MFont* OpenFont(const char* szName, int nHeight) = 0;

	void SetGlobalEvent(MGLOBALEVENTCALLBACK pGlobalEventCallback);

	// IME 관련 함수들
	/// 현재 언어에 따른 기본 폰트 얻기
	const char* GetDefaultFontName(void) const;

	/// 주 언어 ID 얻기
	/// LANG_KOREAN, LANG_JAPANESE, LANG_CHINESE, etc... in "winnt.h"
	int GetPrimaryLanguageIdentifier(void) const;

	/// 부 언어 ID 얻기
	/// 중국어 간체, 번체를 얻기 위해서는 이 정보가 필요
	/// SUBLANG_KOREAN, SUBLANG_CHINESE_TRADITIONAL, SUBLANG_CHINESE_SIMPLIFIED, ... in "winnt.h"
	int GetSubLanguageIdentifier(void) const;

	/// 인디케이터(En, 한, 日, 中 등) 얻기
	const char* GetLanguageIndicatorString(void) const;

	/// Native 언어로 설정되어 있는가?
	bool IsNativeIME(void) const;

	/// WM_IME_NOTIFY의 IMN_OPENCANDIDATE 대응, Candidate List 팝업될 때 호출
	void OpenCandidateList(void);
	/// WM_IME_NOTIFY의 IMN_CLOSECANDIDATE 대응, Candidate List 팝업이 닫힐 때 호출
	void CloseCandidateList(void);

	// Candidate List 관련 함수들
	/// 인덱스값으로 Candidate 문자열 얻기
	const char* GetCandidate(int nIndex) const;
	/// Candidate 문자열 총 개수 얻기
	int GetCandidateCount(void) const;
	/// Candidate 문자열들 중 현재 선택된 문자열 인덱스 얻기
	int GetCandidateSelection(void) const;
	/// 현재 페이지에 보여줄 Candidate 문자열의 첫번째 인덱스 얻기
	int GetCandidatePageStart(void) const;
	/// 한 페이지에 보여줄 Candidate 문자열의 개수 얻기
	int GetCandidatePageSize(void) const;

	/// Candidate List가 보여질 위치 지정하기
	/// p는 조합되는 글자의 시작 위치
	/// 아래쪽 여백이 부족하면 위로 출력해줘야 하므로 위젯의 높이를 알아야 한다.
	void SetCandidateListPosition(MPOINT& p, int nWidgetHeight);
	/// Candidate List 너비
	int GetCandidateListWidth(void);
	/// Candidate List 높이
	int GetCandidateListHeight(void);

	/// Composition 속성 개수 얻기
	DWORD GetCompositionAttributeSize(void) const { return m_nCompositionAttributeSize; }
	/// Composition 속성 얻기
	const BYTE* GetCompositionAttributes(void) const { return m_nCompositionAttributes; }
	//const DWORD* GetCompositionClauses(void) const { return m_dwCompositionClauses; }

	/// 컴포지션 속성 그리기
	/// p는 szComposition[i]의 위치
	int DrawCompositionAttribute(MDrawContext* pDC, MPOINT& p, const char* szComposition, int i);
	/// p는 Composition이 시작하는 위치
	void DrawCompositionAttributes(MDrawContext* pDC, MPOINT& p, const char* szComposition);
	/// 왼쪽에 인디케이터 그리기
	void DrawIndicator(MDrawContext* pDC, MRECT& r);
};

/// Mint가 동작될 화면의 가로 크기 얻기
inline int MGetWorkspaceWidth(void){
	return Mint::GetInstance()->GetWorkspaceWidth();
}
/// Mint가 동작될 화면의 세로 크기 얻기
inline int MGetWorkspaceHeight(void){
	return Mint::GetInstance()->GetWorkspaceHeight();
}

// Sample
void MCreateSample(void);
void MDestroySample(void);

/*
class MUIManager
{
private:
	MIDLResource*	m_pResource;
public:
	MUIManager() { }
	virtual ~MUIManager() { }
//	void Control(string szItem, MWidget* );
};
*/

#ifdef WIN32
#define MMODIFIER_ALT	MOD_ALT
#define MMODIFIER_CTRL	MOD_CONTROL
#define MMODIFIER_SHIFT	MOD_SHIFT
#else
#define MMODIFIER_ALT	1
#define MMODIFIER_CTRL	2
#define MMODIFIER_SHIFT	4
#endif

#define MIsActionKeyPressed(_ActionID)	(Mint::GetInstance()->IsActionKeyPressed(_ActionID))

#define IsHangul(x) ((unsigned char)(x)>127) 

#endif