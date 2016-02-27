#ifndef ZINTERFACEBACKGROUND_H
#define ZINTERFACEBACKGROUND_H

#include "RBspObject.h"
using namespace RealSpace2;

#define LOGIN_SCENE_FIXEDSKY	0			// 하늘에 카메라 고정
#define LOGIN_SCENE_FALLDOWN	1			// 카메라 내려오면서 로고 보임
#define LOGIN_SCENE_FIXEDCHAR	2			// 캐릭터한테 카메라 고정
#define LOGIN_SCENE_SELECTCHAR	3			// 캐릭터 선택 후

class ZInterfaceBackground{
private:
	RBspObject*	m_pLogin;					// 배경
	ZMapDesc* m_pMapDesc;

	rmatrix		m_matWorld;					// 월드 맵

	int			m_nSceneNumber;				// 장면 번호
	
	rvector		m_vCamPosSt;				// 카메라 시작 위치
	rvector		m_vCamPosEd;				// 카메라 종료 위치
	rvector		m_vCamDirSt;				// 카메라 시작 방향
	rvector		m_vCamDirEd;				// 카메라 종료 위치
	rvector		m_vCharPos;					// 캐릭터 위치
	rvector		m_vCharDir;					// 캐릭터 방향

	DWORD		m_dwClock;

//	bool		m_bShowMaietLogo;			// Maiet logo 보임


protected:
	void SetFogState( float fStart, float fEnd, unsigned long int color);


public:
	ZInterfaceBackground( void);
	virtual ~ZInterfaceBackground( void);

	RBspObject*	GetChurchEnd() { return m_pLogin; }
	int GetScene() { return m_nSceneNumber; }
	void SetScene(int nSceneNumber);

	void LoadMesh( void);
	void Free(void);
	void Draw(void);

	void OnUpdate(float fElapsed);
	void OnInvalidate();
	void OnRestore();

	rvector& GetCharPos() { return m_vCharPos; }
	rvector& GetCharDir() { return m_vCharDir; }
};


#endif
