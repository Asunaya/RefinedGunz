#ifndef _RLenzFlare_h
#define _RLenzFlare_h

#pragma once

#include "RBaseTexture.h"

_NAMESPACE_REALSPACE2_BEGIN

//////////////////////////////////////////////////////////////////////////
//  [11/11/2003]
//	2D Lenz Flare
//	Magicbell
//	- source 
//		1. lenz flare형식의 xml 파일 - 예제를 보시오
//		2. lenz flare에 쓰일 텍스쳐들.. ( FLARE_ELEMENT_TYPE당 하나 )
//	- 사용법
//		게임 시작 단계에서 RCreateLenzFlare을 호출한다 - argument : xml 파일 이름
//		RGetLenzFlare를 호출하여 Instance Pointer를 얻는다
//		Render를 호출한다 - argument1 : 광원 위치 argument2: 현재 적용안됨
//			주의) 이때 사물에 가려져 광원이 보이지 않는 처리는 Application에서 해야 함
//					화면밖의 광원은 이 클래스에서 처리될 수 있다
//		게임 종료 단계에서 RDestroyLenzFlare를 호출한다
//
//////////////////////////////////////////////////////////////////////////

//	enum
enum FLARE_ELEMENT_TYPE
{
	FLARE_ELEMENT_SPHERE = 0,
	FLARE_ELEMENT_RING,
	FLARE_ELEMENT_SPOT,
	FLARE_ELEMENT_POLYGON,
	FLARE_ELEMENT_ETC,
	FLARE_ELEMENT_GROW,
	MAX_NUMBER_ELEMENT = 10,
};

#define MAX_LENZFLARE_NUMBER 1
#define MAX_NUMBER_TEXTURE MAX_NUMBER_ELEMENT

//	struct
struct sFlareElement
{
	int iType;
	float width, height;
	DWORD color;
	int iTextureIndex;
};

class RBspObject;

class RLenzFlare
{
protected:	
	//static int			msRef;						// 현재 생성된 렌즈 플레어의 객체 수
	static bool				mbIsReady;				// 사용준비
	static RLenzFlare		msInstance;			// 싱글톤
	static sFlareElement	msElements[MAX_NUMBER_ELEMENT];		
	int								miNumFlareElement;		// 렌즈 플레어에 쓰이는 다각형의 갯수
	int*							miElementOrder;
	static RealSpace2::RBaseTexture		*msTextures[MAX_NUMBER_TEXTURE];
	//static LPDIRECT3DVERTEXBUFFER9		msVB;
	rvector						mLightList[MAX_LENZFLARE_NUMBER];
	int								miNumLight;

protected:
	static	bool	ReadXmlElement(MXmlElement* PNode,char* Path);
	bool		open( const char* pFileName_, MZFileSystem* pfs_ );
	bool		draw( float x_, float y_,				// 위치
					float width_, float height_,		// 크기
					float alpha,							// 불투명도	0~1
					DWORD color_,					// 색
					int	textureIndex_	);				// 텍스쳐 인덱스
	

public:
	static	bool	Create( char* filename_ );
	static	bool	Destroy();
	static	bool	IsReady();
	static	RLenzFlare* GetInstance(){ return &msInstance; 	}

	void		Initialize();									// 플레어 엘레멘트의 순서와 갯수를 결정
	bool		Render( rvector& light_pos,		// 광원
					rvector& centre_,					// 화면 중앙(카메라 위치) 혹은 캐릭터 눈 위치(???)
					RBspObject* pbsp_ );				
	bool		Render( rvector& centre_ , RBspObject* pbsp_ );
	bool		SetLight( rvector& pos_ );
	void		Clear() { miNumLight	= 0;	}
	int			GetNumLight() const { return miNumLight; }
	rvector GetLightPos( int i ) const { return mLightList[i]; }

public:
	RLenzFlare(void);
	~RLenzFlare(void);
};

#ifndef __DEFINED_GLOBAL_LENZFLARE_METHOD__
#define __DEFINED_GLOBAL_LENZFLARE_METHOD__

//	렌즈 플레어 기능을 사용하기 위해 호출
bool	RCreateLenzFlare( char* filename_ );

// 더이상 렌즈 플레어 기능을 사용하지 않음을 선언
bool	RDestroyLenzFlare( );

// 렌즈 플레어가 사용 준비가 되었는지 쿼리
bool	RReadyLenzFlare( ) ;

// 렌즈 플레어 객체 얻어오기
//RLenzFlare*	RGetLenzFlare( bool* result = NULL);
RLenzFlare* RGetLenzFlare();

#endif

_NAMESPACE_REALSPACE2_END

#endif//_RLenzFlare_h