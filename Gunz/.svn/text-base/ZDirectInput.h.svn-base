#ifndef ZDIRECTINPUT_H
#define ZDIRECTINPUT_H

#define DIRECTINPUT_VERSION 0x0800

#include <dinput.h>
#include <windows.h>

struct ZDIBUFFER{
	BYTE nKey;
	bool bPressed;
};

// Immediate Mode : 현재 상태를 얻을수 있는 모드
// Buffered Mode : 현재 발생한 입력을 버퍼로 읽어오는 모드


/// DirectInput Wrapper
class ZDirectInput{
protected:
	HMODULE					m_hD3DLibrary;
	BOOL					m_bInitialized;
	LPDIRECTINPUT8			m_pDI;				///< The DirectInput object         
	LPDIRECTINPUTDEVICE8	m_pKeyboard;		///< The keyboard device 
	BOOL					m_bImmediateMode;

	LPDIRECTINPUTDEVICE8	m_pMouse;			///< mouse device
	unsigned int			m_nMouseButtons;	///> mouse buttons

	LPDIRECTINPUTDEVICE8	m_pJoystick;		///< joystick device
	unsigned int			m_nJoyButtons;		///< joystick buttons
	unsigned int			m_nJoyPovs;			///< joystick povs
	unsigned int			m_nFFAxis;			///< force feedback axis
	bool					m_bForceFeedback;
	LPDIRECTINPUTEFFECT		m_pFFEffect;		///< force feedback effect

#define KEYNAMETABLE_COUNT	256
	char*					m_szKeyNameTable[KEYNAMETABLE_COUNT];

	static BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext );
	static BOOL CALLBACK EnumJoyObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext );

public:
	ZDirectInput(void);
	virtual ~ZDirectInput(void);

	bool CreateDirectInput();
	bool Create(HWND hWnd, BOOL bExclusive=TRUE, BOOL bImmediateMode=TRUE);
	void Destroy(void);

	void OnActivate(bool bActive);

	BOOL IsInitialized()	{ return m_bInitialized; }

	/// 256개의 스캔코드가 눌려있는지(&0x80) 검사 . 사용하지 않음
//	bool GetImmediateData(BYTE ScanCode[256]);

	///////////////////////////////////////////////////////////////////////////////////////
	/// 키보드 

	/// 키보드 버퍼로 읽어오기, 스캔코드 및 Down/Up 플래그 포함, 읽은 버퍼 갯수만큼 리턴한다.
	DWORD GetKeyboardBufferedData(ZDIBUFFER* pBuffer,unsigned int nBuffer);

	const char* GetKeyName(unsigned long int nKey);


	////////////////////////////////////////////////////////////////////////////////////////
	/// 마우스

	// 마우스 버튼 개수 얻어오기
	unsigned int GetMouseButtonCount()		{ return m_nMouseButtons; }

	// 마우스 버퍼로 읽어오기
	DWORD GetMouseBufferedData(int* pSumX,int* pSumY, ZDIBUFFER* pBuffer,unsigned int nBuffer);
	
	// 마우스 데이터 얻어오기
	bool GetImmediateData(DIMOUSESTATE2 *pdims2);



	///////////////////////////////////////////////////////////////////////////////////////
	/// 조이스틱

	// pov 개수 얻어오기
	unsigned int GetJoystickPovCount()		{ return m_nJoyPovs; }
	// 버튼 개수 얻어오기
	unsigned int GetJoystickButtonCount()	{ return m_nJoyButtons; }
	// 조이스틱 상태 얻어오기
	bool GetJoystickData(DIJOYSTATE2* pjs);

	bool SetDeviceForcesXY(int nXForce, int nYForce);

};

#define ISKEYDOWN(_ScanCodeTable, _ScanCode)	((_ScanCodeTable[_ScanCode]&0x80)?true:false)

#endif