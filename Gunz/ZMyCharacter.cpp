/*
last modify : 정동섭 @ 2006/3/16
desc : 무기 사용 키 커스터마이즈 관련
*/

#include "stdafx.h"

#include "ZGameInterface.h"
#include "ZMyCharacter.h"
#include "ZSoundEngine.h"
#include "ZApplication.h"
#include "ZPost.h"
#include "ZActionDef.h"
#include "MProfiler.h"
#include "ZScreenEffectManager.h"
#include "ZConfiguration.h"
#include "ZGameAction.h"
#include "ZModule_HPAP.h"
#include "MDebug.h"
#include "MMath.h"
#include "ZGameConst.h"
#include "ZInput.h"
#include "Rules.h"
#include "HitRegistration.h"
#include "MUtil.h"

#define CHARGE_SHOT		// 모아서 강베기 가능

//#define UNLIMITED_JUMP						// 이거 해놓으면 무한점프

#define GUARD_DURATION		2.f				// 방어 지속시간

#define TUMBLE_DELAY_TIME	.5f				// 연속덤블링 사이의 딜레이
#define AIR_MOVE			0.05f			// 공중에서 방향을 바꾸기는 힘들다. 땅에서의 이만큼만 움직인다..

#define JUMP_QUEUE_TIME		0.3f			// 이시간정도 점프를 미리 누를수있다.

#define JUMP2_WALL_VELOCITY 300.f			// 이중점프할때 벽방향으로 뛰어나가는 속도
#define JUMP2_VELOCITY		1400.f			// 이중점프로 올라가는 속도 (높이와 관련있음)

#define JUMP_VELOCITY		900.f			// 일반점프로 올라가는 속도 (높이와 관련있음)

#define WALL_JUMP2_SIDE_VELOCITY	700.f	// 벽점프중 점프속도 (양옆방향)
	
#define WALL_JUMP_VELOCITY	350.f			// 벽점프 올라가는 속도

#define BLAST_VELOCITY		1700.f			// 띄워지는 높이

#define SHOT_QUEUE_TIME		0.4f			// 이시간정도 칼질을 미리 누를수있다.

#define COLLISION_POSITION rvector(0,0,60)	// 충돌체크할 지점.

#define CAFACTOR_DEC_DELAY_TIME	0.2f			// Controllability 감소 딜레이

MImplementRTTI(ZMyCharacter, ZCharacter);

#undef min
ZMyCharacter::ZMyCharacter() : LastDamagedTime(std::numeric_limits<float>::lowest())
{
	m_fCAFactor = 1.0f;
	m_fElapsedCAFactorTime = 0.0f;

#ifdef _DEBUG
	m_bGuardTest=false;
#endif
	InitStatus();
	m_Position=rvector(0,0,500);

	InitRound();//변수 클리어 하기 위해 사용

	m_fWallJumpTime = 0.f;
	m_nTumbleDir = 0;
	m_fHangTime = 0.f;
	m_nWallJump2Dir = 0;
	
//	m_fSkillTime = 0.f;
	m_fDropTime = 0.f;

	m_bSniferMode = false;

	m_bPlayDone = false;
	m_bPlayDone_upper = false;

	m_bMoveLimit = false;

	m_bReserveDashAttacked = false;
	m_vReserveDashAttackedDir = rvector(0.f,0.f,0.f);
	m_fReserveDashAttackedTime = 0.f;
	m_uidReserveDashAttacker = MUID(0,0);

//	m_bForceDamage = false;
	m_bGuardKey = false;
	m_bGuardByKey = false;
	m_pModule_HPAP->SetCheckSecurity(true);
	m_pModule_HPAP->SetRealDamage(true);

	m_pModule_Movable->Active(false);
	//m_pModule_Movable->SetCollision(m_Collision.fRadius, m_Collision.fHeight);

	// 충돌처리값
	m_Collision.fRadius = CHARACTER_RADIUS;
	m_Collision.fHeight = CHARACTER_HEIGHT;
}

ZMyCharacter::~ZMyCharacter()
{
#ifndef _PUBLISH
	ZModule *pMod = GetModule(ZMID_SKILLS);
	if(pMod) {
		RemoveModule(pMod);
		delete pMod;
	}
#endif

	while(m_DelayedWorkList.size())
	{
		ZDELAYEDWORKITEM *pItem = *m_DelayedWorkList.begin();
		delete pItem;
		m_DelayedWorkList.pop_front();
	}
}

void ZMyCharacter::InitRound()
{
	ZCharacter::InitRound();

	for(int i=0;i<MMCIP_END;i++)
		m_fNextShotTimeType[i] = 0.f;

	m_bReserveDashAttacked = false;
}

void ZMyCharacter::InitSpawn()
{
	for(int i=0;i<MMCIP_END;i++)
		m_fNextShotTimeType[i] = 0.f;

	m_bReserveDashAttacked = false;
}

//BSPVERTEX *pJumpVertices=NULL;

//rvector g_shotgun_line[2];

void ZMyCharacter::OnDraw()
{
	if (m_bSniferMode) return;

	ZCharacter::OnDraw();
/*
	RDrawLine(g_shotgun_line[0],g_shotgun_line[1],0xffff0000);
	
	rmatrix m;
	D3DXMatrixIdentity(&m);

	m._41 = g_shotgun_line[0].x;
	m._42 = g_shotgun_line[0].y;
	m._43 = g_shotgun_line[0].z;

	draw_box(&m,rvector(5,10,5),rvector(-5,0,-5),0xffffffff);
*/
	/*
	RGetDevice()->SetTexture(0,NULL);
	RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
	RGetDevice()->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );

	if(pJumpVertices)
	{
		for(int i=0;i<3;i++)
		{
			RDrawLine(*pJumpVertices[i].Coord(),*pJumpVertices[(i+1)%3].Coord(),0xffffff80);
		}

	}
	*/


	/*
	// 임시 효과
	static RParticles *pParticles = NULL;
	if(!pParticles) {
		pParticles = RGetParticleSystem()->AddParticles("shademap.bmp",5.f);
	}

	if(m_bCharged)
	{
#define MAXVELOCITY	50.f
#define COUNT	4

		for(int i=0;i<COUNT;i++)
		{
			RParticle *pp=new RParticle;
			pp->ftime=0;
			pp->position=m_Position+rvector(0,0,100.f);
			float fa=2*pi*RANDOMFLOAT,fb=2*pi*RANDOMFLOAT;
			pp->velocity=rvector(MAXVELOCITY*sinf(fa)*cosf(fb),MAXVELOCITY*cosf(fa)*cosf(fb),150.f+MAXVELOCITY*sinf(fb));
			pp->accel=rvector(0,0,-300.f);
			pParticles->push_back(pp);
		}
	}
	*/

	/*
	if(m_bCharging)
	{
#define MAXVELOCITY	50.f
#define COUNT	4

		struct RConvParticle : public RParticle {
			rvector vConvPos;
			virtual void Update(float fTimeElapsed) {

				rvector diff = vConvPos - position;
				Normalize(diff);

				velocity=diff*1000.f;

				position+=velocity*fTimeElapsed;

				ftime+=fTimeElapsed;
			}
		};

		for(int i=0;i<COUNT;i++)
		{
			rvector vWPos;
			GetWeaponTypePos(weapon_dummy_muzzle_flash,&vWPos);//무기총구위치

			RConvParticle *pp=new RConvParticle;
			pp->vConvPos = vWPos;
			pp->ftime=0;
			float fa=2*pi*RANDOMFLOAT,fb=2*pi*RANDOMFLOAT;

#define MAX_RANGE	100.f
			pp->position=m_Position+rvector(0,0,100.f) + MAX_RANGE*rvector(sinf(fa)*cosf(fb),cosf(fa)*cosf(fb),sinf(fb));
			pp->velocity=rvector(0,0,0);
//			pp->accel=rvector(0,0,-300.f);
			pParticles->push_back(pp);
		}
	}
	//*/
}

void ZMyCharacter::ProcessInput(float fDelta)
{
	if (ZApplication::GetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_PREPARE) return;
	if (m_bInitialized==false) return;
	if ( ZGetGame()->IsReservedSuicide())			// 자살 예정인 경우 움직이지 못하게 한다
		return;

	UpdateButtonState();

	ZCombatInterface* pCombatInterface = ZGetGameInterface()->GetCombatInterface();
	if (pCombatInterface && pCombatInterface->IsChat())
	{
		ReleaseLButtonQueue();	// 칼질하다 잠시 멈추는 현상때문에 추가
		return;
	}

	// 이동을 제한하고 싶은경우..
/*
	bool limit = false;

	if(m_pVMesh) {

		if( m_pVMesh->GetSelectWeaponType() == z_wd_machinegun) {//머신건..

			int nParts = (int)GetItems()->GetSelectedWeaponParts();

				 if(nParts==MMCIP_MELEE)		nParts = ZCWT_MELEE;
			else if(nParts==MMCIP_PRIMARY)		nParts = ZCWT_PRIMARY;
			else if(nParts==MMCIP_SECONDARY)	nParts = ZCWT_SECONDARY;
			else if(nParts==MMCIP_CUSTOM1)		nParts = ZCWT_CUSTOM1;
			else if(nParts==MMCIP_CUSTOM2)		nParts = ZCWT_CUSTOM2;
			else {// 설마~
				return;
			}

			//다음 공격 시간보다 작다면 지금 공격중이다~

			if( m_fNextShotTimeType[nParts]+0.3f > g_pGame->GetTime() ) {
				if(m_bLand) {
//					GetVelocity() = rvector(0,0,0);
//					m_Accel = rvector(0,0,0);
					limit = true;
//					return;
				}
			}
		}
	}

	if(limit)	m_bMoveLimit = true;
	else		m_bMoveLimit = false;
*/
	static float rotatez=0.f,rotatex=9*pi/10.f;

	m_Accel=rvector(0,0,0);

	rvector right;
	rvector forward=RCameraDirection;
	forward.z=0;
	Normalize(forward);
	CrossProduct(&right,rvector(0,0,1),forward);

	// 움직임 및 점프 처리
	// 발이 땅에 닿아 있을때만 해야 한다.

	// 죽거나 누워있을때는 입력불가
	if (!IsDie() && !m_bStun && !m_bBlastDrop && !m_bBlastStand)
	{	// blast fall 상태일때는 점프키 입력이 가능하다.
		// 벽점프, 덤블링중에는 가속불가...
		if(!m_bWallJump && !m_bTumble && !m_bSkill && !m_bMoveLimit && 
			!m_bBlast && !m_bBlastFall && !m_bBlastAirmove && !m_bCharging && 
			!m_bSlash && !m_bJumpSlash && !m_bJumpSlashLanding)
		{
			if(ZIsActionKeyPressed(ZACTION_FORWARD)==true)	m_Accel+=forward;
			if(ZIsActionKeyPressed(ZACTION_BACK)==true)		m_Accel-=forward;
			if(ZIsActionKeyPressed(ZACTION_LEFT)==true)		m_Accel-=right;
			if(ZIsActionKeyPressed(ZACTION_RIGHT)==true)	m_Accel+=right;
		}


		m_bBackMoving = ZIsActionKeyPressed(ZACTION_BACK);
		/*
		m_bForwardMoving = MIsActionKeyPressed(ZACTION_FORWARD);
		m_bLeftMoving = MIsActionKeyPressed(ZACTION_LEFT);
		m_bRightMoving = MIsActionKeyPressed(ZACTION_RIGHT);
		*/

		Normalize(m_Accel);

		float fRatio = GetMoveSpeedRatio();

		// 날으는 동안은 컨트럴이 힘들다.
		if(!m_bLand) {
			if( Magnitude(rvector(GetVelocity().x,GetVelocity().y,0)) < RUN_SPEED * fRatio )
				m_Accel*=AIR_MOVE;
			else
				m_Accel*=0;
		}

		bool bWallJump=false;
		int nWallJumpDir=-1;

		if( ZIsActionKeyPressed(ZACTION_JUMP) == true )
		{
			if(m_bReleasedJump && !m_bLimitJump)
			{
//				if(UsingStamina(ZUS_Jump)) {
				
					m_fLastJumpPressedTime=g_pGame->GetTime();
					m_bJumpQueued=true;
					m_bWallJumpQueued=true;
//					m_bWallJumpQueued=MEvent::GetRButtonState();	// 오른버튼+Jump 해야 벽타기
					m_bReleasedJump=false;
//				}
			}
		}else
			m_bReleasedJump=true;

		if(m_bJumpQueued && (g_pGame->GetTime()-m_fLastJumpPressedTime>JUMP_QUEUE_TIME))
			m_bJumpQueued=false;

		if(m_bJumpQueued && !m_bTumble && !m_bDrop && !m_bShot && !m_bShotReturn && !m_bSkill && 
			!m_bBlast && !m_bBlastFall && !m_bBlastDrop && !m_bBlastStand && !m_bBlastAirmove &&
			!m_bSlash && !m_bJumpSlash && !m_bJumpSlashLanding)
		{
			if(!m_bWallJump && !m_bGuard && !m_bLimitWall)
			{
				// 벽점프 가능한지 판단한다.
				// 앞으로 갈때만 가능하다. 그리고 올라가고있을때만.

				if(m_bWallJumpQueued && DotProduct(GetVelocity(),m_Direction)>0 && GetVelocity().z>-10.f)
				{
					rvector pickorigin,pickto,dir;
					rvector front=m_Direction;
					Normalize(front);

					// 앞방향으로의 벽점프~  앞방향은 땅에 있을때만 가능.
					dir=front;
					dir.z=0;
					Normalize(dir);

					float fRatio = GetMoveSpeedRatio();

					if(m_bLand && DotProduct(GetVelocity(),dir)>RUN_SPEED * fRatio * .8f)	// 땅에 있나.. 속도가 맞나..
					{
						pickorigin=m_Position;	// 발디딜곳이 있는지 허리부근 머리부근.. 두군데 검사한다..

						RBSPPICKINFO bpi1,bpi2;
						bool bPicked1=ZGetGame()->GetWorld()->GetBsp()->Pick(pickorigin+rvector(0,0,100),dir,&bpi1);
						bool bPicked2=ZGetGame()->GetWorld()->GetBsp()->Pick(pickorigin+rvector(0,0,180),dir,&bpi2);
						if(bPicked1 && bPicked2)
						{
							rvector backdir=-dir;
							float fDist1=Magnitude(pickorigin+rvector(0,0,100)-bpi1.PickPos);
							float fDist2=Magnitude(pickorigin+rvector(0,0,180)-bpi2.PickPos);

							// 거리와 각도가 맞아야 한다
							if(fDist1<120 && (D3DXPlaneDotNormal(&bpi1.pInfo->plane,&backdir)>cos(10.f/180.f*pi)) && 
								fDist2<120 && (D3DXPlaneDotNormal(&bpi2.pInfo->plane,&backdir)>cos(10.f/180.f*pi)))
							{
								bWallJump=true;
								nWallJumpDir=1;
							}
						}
					}

					// 왼쪽/오른쪽방향의 벽점프 ( i=0 왼쪽, i=1 오른쪽 )
					rvector right;
					CrossProduct(&right,rvector(0,0,1),front);

					pickorigin=m_Position+rvector(0,0,150);

					for(int i=0;i<2;i++)
					{
						dir = (i==0) ? -right : right;

						RBSPPICKINFO bpi;
						bool bPicked=ZGetGame()->GetWorld()->GetBsp()->Pick(pickorigin,dir,&bpi);
						if(bPicked)
						{
							rvector backdir=-dir;
							float fDist=Magnitude(pickorigin-bpi.PickPos);
							rvector normal=rvector(bpi.pInfo->plane.a,bpi.pInfo->plane.b,bpi.pInfo->plane.c);
							float fDot=DotProduct(normal,backdir);

							/*
							rvector wallupdir,jumpdir;
							CrossProduct(&wallupdir,dir,normal);
							Normalize(wallupdir);
							CrossProduct(&jumpdir,wallupdir,normal);
							*/
							rvector wallright,jumpdir;
							CrossProduct(&wallright,rvector(0,0,1),normal);
							jumpdir = (i==0) ? -wallright : wallright;

							float fRatio = GetMoveSpeedRatio();

							// 거리와 속도가 맞고, 각도가 맞아야 한다.
							if(fDist<100.f && DotProduct(GetVelocity(),jumpdir)>RUN_SPEED * fRatio *.8f &&
								fDot>cos(55.f/180.f*pi) && fDot<cos(25.f/180.f*pi) && DotProduct(jumpdir,dir)<0)
							{
								// 대략 벽밟고 3미터쯤 달려간 뒤에도 같은 평면을 밟을수 있어야 한다.
								rvector neworigin=pickorigin+300.f*jumpdir;

								RBSPPICKINFO bpi;
								bPicked=ZGetGame()->GetWorld()->GetBsp()->Pick(neworigin,dir,&bpi);
								if(bPicked && fabsf(Magnitude(bpi.PickPos-neworigin)-fDist)<10)
								{

									// 대략 3미터쯤까지는 걸리는게 없어야 한다.

									// origin 에서 diff 만큼 움직일때, 벽을 검사해서 움직일수 있는 만큼만 diff 를 조절한다.
									//bool CheckWall(rvector &origin,rvector &diff,rvector &velocity,rvector &adjustednormal,float fMargin);
									
									rvector targetpos=pickorigin+300.f*jumpdir;
									bool bAdjusted=ZGetGame()->GetWorld()->GetBsp()->CheckWall(pickorigin,targetpos,CHARACTER_RADIUS-5,60);
									if(!bAdjusted)
									{
										bWallJump=true;
										nWallJumpDir= (i==0) ? 0 : 2 ; // ( 왼쪽 : 오른쪽 )

										SetTargetDir(jumpdir);
										float speed=Magnitude(GetVelocity());
										SetVelocity(jumpdir*speed);
									}
								}
							}
						}
					}
				}
			}


			// 최소한 0.2초 전에 어딘가 부딫혔고, 천장이 아니고..  일정각도가 맞아야 하며, 
			// 마지막 이중점프후 일정시간이 지낫으면 이후에는 이중점프~


			rvector PickedNormal=rvector(0,0,0);
			rvector pickorigin=m_Position+rvector(0,0,90);
			rvector velocity = GetVelocity();
			RBSPPICKINFO bpi;
			bool bPicked=ZGetGame()->GetWorld()->GetBsp()->Pick(pickorigin,velocity,&bpi);
			if(bPicked)
				PickedNormal=rvector(bpi.pInfo->plane.a,bpi.pInfo->plane.b,bpi.pInfo->plane.c);

			float fDotJump2=DotProduct(PickedNormal,m_Direction);
			float fProjSpeed = DotProduct(PickedNormal,GetVelocity());
			
			rvector characterright;
			CrossProduct(&characterright,m_Direction,rvector(0,0,1));
			float fDotJump2right=DotProduct(PickedNormal,characterright);

			bool bJump2=(g_pGame->GetTime()-m_pModule_Movable->GetAdjustedTime()<0.2f 
				&& Magnitude(pickorigin-bpi.PickPos)<100.f
				&& DotProduct(rvector(0,0,-1),PickedNormal)<0.1f
				&& (fDotJump2>.8f || fDotJump2<-.8f || fDotJump2right>.8f || fDotJump2right<-.8f)
				&& g_pGame->GetTime()-m_fJump2Time>.5f)
				&& fProjSpeed < -100.f
				&& GetDistToFloor() > 30.f
				&& !m_bWallJump && !bWallJump && !m_bLand && !m_bGuard;

			// 벽점프 중이면 마무리점프~ (시작한지 .7초 뒤에 가능)
			if(m_bWallJump && !m_bWallJump2 && (m_fWallJumpTime+.4f<g_pGame->GetTime()))
			{
 				WallJump2();
			}else
			if(bWallJump)		// 벽점프이면
			{
				m_bJumpQueued=false;
				m_bWallJump=true;
				m_nWallJumpDir=nWallJumpDir;
				m_fWallJumpTime=g_pGame->GetTime();
				m_bWallJump2=false;
				m_bWallHang = false;

				if(nWallJumpDir==1)
				{
//						m_Position+=rvector(0,0,3);
					//SetVelocity(rvector(0,0,400);
					SetVelocity(0,0,0);

				}
				else
				{
					if(m_bLand) 
						AddVelocity(rvector(0,0,WALL_JUMP_VELOCITY));
				}
				m_bLand=false;

			}else
			if(bJump2)			// 이중점프
			{
				m_bJumpQueued=false;
//				pJumpVertices = & bpi.pNode->pVertices[bpi.nIndex*3];

				// 벽방향의 속도성분을 없앤다.
				Normalize(PickedNormal);
				float fAbsorb=DotProduct(PickedNormal,GetVelocity());
				rvector newVelocity = GetVelocity();
				newVelocity-=fAbsorb*PickedNormal;

				newVelocity*=1.1f; // 가속받는다.

				newVelocity+=PickedNormal*JUMP2_WALL_VELOCITY;
				newVelocity.z=JUMP2_VELOCITY;
				SetVelocity(newVelocity);

				m_fJump2Time=g_pGame->GetTime();

				m_bLand=false;
				m_bWallHang=false;
//				m_bJumpShot=false;
//				mlog("2 jumpshot false\n");

				// 점프칼질중에 점프하면 모션을 생략한다
				if(!m_bJumpShot)
					m_bWallJump2=true;
				m_bPlayDone=false;
				if(fDotJump2>.8f)
					m_nWallJump2Dir=0;
				else 
				if(fDotJump2<-.8f)
					m_nWallJump2Dir=1;
				else
				if(fDotJump2right<-.8f)
					m_nWallJump2Dir=2;
				else 
				if(fDotJump2right>.8f)
					m_nWallJump2Dir=3;

				if(m_AniState_Upper==ZC_STATE_UPPER_SHOT)	// 단검 애니메이션일경우
					SetAnimationUpper(ZC_STATE_UPPER_NONE);	// 애니메이션 캔슬

			}else
			if(m_bWallHang)
			{
				if(m_bHangSuccess)	// 매달린 상태에서의 점프
				{
					/*
					rvector PickedNormal=rvector(0,0,0);
					rvector pickorigin=m_Position+rvector(0,0,90);
					RBSPPICKINFO bpi;
					bool bPicked=ZGetGame()->GetWorld()->GetBsp()->Pick(pickorigin,pickorigin+m_Direction,&bpi);
					if(bPicked)
						PickedNormal=rvector(bpi.pInfo->plane.a,bpi.pInfo->plane.b,bpi.pInfo->plane.c);
					*/
					m_bJumpQueued=false;

					rvector PickedNormal=-m_Direction;PickedNormal.z=0;

					if(ZIsActionKeyPressed(ZACTION_FORWARD))
					{
						m_nWallJump2Dir=6;
						SetVelocity(PickedNormal*50.f);
					}
					else
					{
						m_nWallJump2Dir=7;
						AddVelocity(PickedNormal*300.f);
					}
					AddVelocity(rvector(0,0,1400));

					/*
					GetVelocity().x=0;
					GetVelocity().y=0;

					GetVelocity()+=PickedNormal*JUMP2_WALL_VELOCITY;
					GetVelocity().z=JUMP2_VELOCITY*1.5;
					*/

					m_fJump2Time=g_pGame->GetTime();

					m_bLand=false;
					m_bWallHang=false;
					
					m_bWallJump2=true;
					m_bPlayDone=false;
//					m_nWallJump2Dir=1;
				}
			}else

#ifndef UNLIMITED_JUMP
			if(m_bLand)		// 그렇지 않으면 일반점프이다.
#endif
			{
				m_bJumpQueued=false;

				rvector right;
				rvector forward=m_Direction;
				CrossProduct(&right,rvector(0,0,1),forward);

				rvector vel=rvector(GetVelocity().x,GetVelocity().y,0);
				float fVel=Magnitude(vel);

				rvector newVelocity = vel*0.5f + m_Accel*fVel*.45f + rvector(0,0,JUMP_VELOCITY);
				SetVelocity(newVelocity);
				m_bLand=false;

				m_fJump2Time=g_pGame->GetTime();
			}
		}
	}

	m_Accel*=7000.f*fDelta;
	AddVelocity(m_Accel);

	// 나머지 입력을 처리한다
	ProcessGuard();
	ProcessShot();
	ProcessGadget();

}

//// 상대방공격이 막혀서 반격기 ( 강베기 ) 를 먹일 수 있는 타이밍인지 리턴
//bool ZMyCharacter::IsCounterAttackable()
//{
//	for (ZCharacterManager::iterator itor = ZGetCharacterManager()->begin();
//		itor != ZGetCharacterManager()->end(); ++itor)
//	{
//		ZCharacter* pTar = (*itor).second;
//
//		if(pTar==this) continue;
//
//		bool bCheck = true;
//		// 팀플레이이고 같은팀이면 체크하지 않는다
//		if (ZApplication::GetGame()->GetMatch()->IsTeamPlay() && IsTeam( pTar ) ) {
//			bCheck = false;
//		}
//
//		if(bCheck)
//		{
//			if(pTar->m_AniState_Lower==ZC_STATE_DAMAGE_BLOCKED) {
//                
//				rvector dir = pTar->GetPosition()-m_Position;
//				float fDist = Magnitude(dir);
//				Normalize(dir);
//				if(DotProduct(dir,m_Direction)>.5f && fDist<200.f) {
//					// 적절하게 공격을 넣을수 있는 사람이 하나라도 있으면 가능하다고 리턴
//					return true;
//				}
//			}
//		}
//	}
//
//	return false;
//}

void ZMyCharacter::OnShotMelee()
{
	/*
	ZMeleeDesc* pMeleeDesc = (ZMeleeDesc*)(pSelectedItem->GetDesc());
	DWORD nWeaponDelay = pMeleeDesc->m_nDelay * 100;
	if (nElapsedTime < nWeaponDelay) return;
	nElapsedTime = 0;
	*/
	if(m_bSkill || m_bStun || m_bShotReturn || m_bJumpShot || m_bGuard ||
		m_bSlash || m_bJumpSlash || m_bJumpSlashLanding ) return;		 // || m_bTumble

	if(m_bShot && !m_bPlayDone)
		return;

	// 단검
	if( m_pVMesh->m_SelectWeaponMotionType==eq_wd_dagger ||
		m_pVMesh->m_SelectWeaponMotionType==eq_ws_dagger ) { // dagger

		if(m_AniState_Upper==ZC_STATE_UPPER_SHOT)
			return;

		if(m_bCharged/* || IsCounterAttackable()*/) 
		{
			if(GetVelocity().z>100.f || GetDistToFloor() > 80.f)
				JumpChargedShot();
			else
				ChargedShot();
			return;
		}

		SetAnimationUpper(ZC_STATE_UPPER_SHOT);
		m_bWallJump=false;
		m_bWallJump2=false;
#ifdef CHARGE_SHOT
		m_bEnterCharge=true;
#endif
	} else { // katana , sword , blade

//		int nShotCount= (m_pVMesh->m_pMesh->m_ani_mgr.GetAnimation("attack5") != NULL) ? 5 : 4; // 하드코드 -_-;

//		int nShotCount = 5;//남녀 모두 5단계로 바뀜~

		if(!m_bShot)
			m_nShot=0;

		// 5번째 칼질 봉인
		int nShotCount = 4;

		// 2005.4.4 대검도 4번째 칼질이 들어갑니다.
		//if( m_pVMesh->m_SelectWeaponMotionType == eq_wd_sword ) // blade
		//{
		//	nShotCount = 3;
		//}

		if(m_nShot==nShotCount) {
			return;
		}

		// 어느정도 높이 있으면 고공베기가 가능.
		if( !m_bJumpShot && (GetVelocity().z>100.f || GetDistToFloor() > 80.f) )
		{
			if(m_bCharged) 
			{
				JumpChargedShot();
				return;
			}

			m_bShot=false;
			m_bShotReturn=false;
			m_bWallJump=false;
			m_bWallJump2=false;
			m_bJumpShot=true;
			m_bPlayDone=false;
			
			if(IsRunWall() && IsMeleeWeapon() ) {
				m_nJumpShotType = 1;
			}
			else {
				m_nJumpShotType = 0;
			}

			m_fNextShotTimeType[MMCIP_MELEE] = g_pGame->GetTime() + 0.8f;

		} else {

			// 공중이면서 고공베기를 못하는 조건이면 칼질 안함
			// 여자 마지막 칼질은 공중에서 하므로 예외
			if(!m_bLand && GetDistToFloor()>20.f) {
				if(m_nShot < 2) {
					ReleaseLButtonQueue();
					return;	
				}
			}

			if(m_bCharged/* || IsCounterAttackable()*/) {
				ChargedShot();
				return;
			}

			m_bShot=true;
			m_bShotReturn=false;
			m_bPlayDone=false;

#ifdef CHARGE_SHOT
			if(m_nShot==0)
				m_bEnterCharge=true;
#endif

			m_nShot=m_nShot % nShotCount + 1;
		}
	}

	m_bTumble=false;
	ReleaseLButtonQueue();

	int sel_type = GetItems()->GetSelectedWeaponParts();

	// 첫번째 칼질은 딜레이가 있다
	if(m_nShot==1 && !m_bJumpShot) {
		m_b1ShotSended=false;
		m_f1ShotTime=g_pGame->GetTime();
	}else {
//			mlog("zpost_melee !\n");
		if (g_Rules.IsVanillaMode())
		{
			ZPostShotMelee(g_pGame->GetTime(), m_Position, m_nShot);
			AddDelayedWork(g_pGame->GetTime() + 0.2f, ZDW_RECOIL);
		}
		else
		{
			AddDelayedWork(g_pGame->GetTime() + 0.1 + max(m_nShot - 1, 0) * .05, ZDW_RG_SLASH, (void *)m_nShot);
		}
	}
}

void ZMyCharacter::ReleaseLButtonQueue()
{
	m_bLButtonQueued = false;
}

void ZMyCharacter::OnShotRange()
{
	ZItem* pSelectedItem = GetItems()->GetSelectedWeapon();

	if( pSelectedItem==NULL ) 
	{
		mlog("ZMyCharacter::OnShotRange pSelectedItem == NUL\nL");
		return;
	}

	// 스나이퍼총은 스나이퍼 모드일때만 쏠 수 있다.
	if ((pSelectedItem->GetDesc()->m_nWeaponType == MWT_SNIFER) && (!m_bSniferMode))
	{
		return;
	}

	// 총알 체크
	if (pSelectedItem->GetBulletAMagazine() <= 0) {
		
		return;
	}

	// 총으로 더블링 중에는 상체 섞는 모션이 안나오게..

	if( m_AniState_Lower != ZC_STATE_LOWER_TUMBLE_FORWARD && 
		m_AniState_Lower != ZC_STATE_LOWER_TUMBLE_BACK &&
		m_AniState_Lower != ZC_STATE_LOWER_TUMBLE_RIGHT &&
		m_AniState_Lower != ZC_STATE_LOWER_TUMBLE_LEFT )
	// 총질 애니메이션
		SetAnimationUpper(ZC_STATE_UPPER_SHOT);

	MPOINT Cp = MPOINT(0, 0);
	ZPICKINFO zpi;
	rvector pickpos;

	ZCombatInterface* ci=ZApplication::GetGameInterface()->GetCombatInterface();

	if (ci)	{

		Cp = ci->GetCrosshairPoint();

		rvector pos,dir;
		RGetScreenLine(Cp.x,Cp.y,&pos,&dir);

		rvector mypos=m_Position+rvector(0,0,100);
		rplane myplane;
		D3DXPlaneFromPointNormal(&myplane,&mypos,&dir);

		rvector checkpos,checkposto=pos+10000.f*dir;
		D3DXPlaneIntersectLine(&checkpos,&myplane,&pos,&checkposto);

		if(g_pGame->Pick(this,checkpos,dir,&zpi,RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSBULLET))
		{
			if(zpi.bBspPicked)
				pickpos=zpi.bpi.PickPos;
			else
				if(zpi.pObject)
					pickpos=zpi.info.vOut;
		}
		else
		{
			pickpos=pos+dir*10000.f;
		}
	}

	bool skip_controllability = false;

	rvector vWeaponPos;
	rvector dir;

	if(CheckWall(vWeaponPos)) {//벽속에 총구 박힌 경우
		skip_controllability = true;
	}
	else {// 아니라면 예전..한가운데서
	}

	dir = pickpos-vWeaponPos;
	Normalize(dir);

	MMatchWeaponType wtype = MWT_NONE;
		
	if(pSelectedItem->GetDesc())
		wtype = pSelectedItem->GetDesc()->m_nWeaponType;

	u32 seed = reinterpret<u32>(g_pGame->GetTime());

	if( ( wtype == MWT_SHOTGUN ) || ( wtype == MWT_SAWED_SHOTGUN ) )
	{
		int sel_type = GetItems()->GetSelectedWeaponParts();

		rvector vTo = pickpos;

		rvector nPos = m_pVMesh->GetBipTypePosition(eq_parts_pos_info_Spine1);
		rvector nDir = pickpos-nPos;
		Normalize(nDir);

		if( skip_controllability || (vWeaponPos.z <  m_Position.z + 20.f ))
		{
			vWeaponPos = nPos + 30.f * nDir;
			vTo = nPos + 10000.f * nDir;
		}

		ZPostShot(g_pGame->GetTime(),vWeaponPos, vTo,sel_type);
	}
	else
	{
		rvector r;

		if (pSelectedItem->GetDesc() != NULL && !skip_controllability)
		{
			// 반동력을 계산한다
			CalcRangeShotControllability(r, dir, pSelectedItem->GetDesc()->m_nControllability, seed);
		}
		else
		{
			r = dir;
		}

		int sel_type = GetItems()->GetSelectedWeaponParts();

		ZPostShot(g_pGame->GetTime(),vWeaponPos, vWeaponPos+10000.f*r,sel_type);
	}
}

void ZMyCharacter::CalcRangeShotControllability(rvector& vOutDir, const rvector& vSrcDir, int nControllability, u32 Seed)
{
	::CalcRangeShotControllability(vOutDir, vSrcDir, nControllability, Seed, m_fCAFactor);
	IncreaseCAFactor();
}

void ZMyCharacter::IncreaseCAFactor()
{
	m_fElapsedCAFactorTime = 0.0f;
	m_fCAFactor += GetControllabilityFactor();
	if (m_fCAFactor > 1.0f) m_fCAFactor = 1.0f;
}

void ZMyCharacter::OnShotItem()
{
	ZItem* pSelectedItem = GetItems()->GetSelectedWeapon();

	if (pSelectedItem==NULL || pSelectedItem->GetBulletAMagazine() <= 0) {
		return;
	}

	SetAnimationUpper(ZC_STATE_UPPER_SHOT);

	int type = ZC_WEAPON_SP_ITEMKIT;

	rvector pos = rvector(0.0f,0.0f,0.0f);
	int sel_type = GetItems()->GetSelectedWeaponParts();

	// 아이템킷일 경우
	if(pSelectedItem->GetDesc()->m_nWeaponType==MWT_MED_KIT ||
		pSelectedItem->GetDesc()->m_nWeaponType==MWT_REPAIR_KIT ||
		pSelectedItem->GetDesc()->m_nWeaponType==MWT_BULLET_KIT ||
		  pSelectedItem->GetDesc()->m_nWeaponType==MWT_FOOD )
	{
		pos = m_Position + (m_Direction * 150);
		pos.z = m_Position.z;

		rvector vWeapon;

		vWeapon = m_Position + rvector(0,0,130) + (m_Direction * 50);

		rvector nPos = m_pVMesh->GetBipTypePosition(eq_parts_pos_info_Spine1);
		rvector nDir = vWeapon - nPos;

		Normalize(nDir);

		RBSPPICKINFO bpi;
		if(ZGetWorld()->GetBsp()->Pick(nPos,nDir,&bpi))	{
			if(D3DXPlaneDotCoord(&(bpi.pInfo->plane),&vWeapon)<0) {
				vWeapon = bpi.PickPos - nDir;
			}
		}

		pos = vWeapon;

		rvector velocity = (GetVelocity() * 0.5f) + m_TargetDir*100;
		ZPostShotSp(g_pGame->GetTime(),pos,velocity,ZC_WEAPON_SP_ITEMKIT,sel_type);
		return;
	}

	rvector vWeaponPos;
	CheckWall(vWeaponPos);

	MPOINT Cp = MPOINT(0, 0);
	ZPICKINFO zpi;
	rvector pickpos;

	ZCombatInterface* ci=ZApplication::GetGameInterface()->GetCombatInterface();


	if (ci) {

		Cp = ci->GetCrosshairPoint();

		rvector pos,dir;
		RGetScreenLine(Cp.x,Cp.y,&pos,&dir);

		rvector mypos=m_Position+rvector(0,0,100);
		rplane myplane;
		D3DXPlaneFromPointNormal(&myplane,&mypos,&dir);

		rvector checkpos,checkposto=pos+100000.f*dir;
		D3DXPlaneIntersectLine(&checkpos,&myplane,&pos,&checkposto);

		if(g_pGame->Pick(this,checkpos,dir,&zpi))
		{
			if(zpi.bBspPicked)
				pickpos=zpi.bpi.PickPos;
			else
				if(zpi.pObject)
					pickpos=zpi.info.vOut;
		}
		else
		{
			pickpos=pos+dir*10000.f;
		}
	}

	rvector v1;
	GetWeaponTypePos(weapon_dummy_muzzle_flash,&v1);

	if(zpi.bBspPicked && D3DXPlaneDotCoord(&(zpi.bpi.pInfo->plane),&v1)<0)
		v1=m_Position+rvector(0,0,110);

	rvector dir = pickpos - v1;

	Normalize(dir);
	ZPostShotSp(g_pGame->GetTime(),v1,dir,type,sel_type);
}

void ZMyCharacter::OnShotCustom()
{
	ZItem* pSelectedItem = GetItems()->GetSelectedWeapon();

	if (pSelectedItem==NULL || pSelectedItem->GetBulletAMagazine() <= 0) {
		return;
	}

	MMatchItemDesc* pCustomDesc = pSelectedItem->GetDesc();
	DWORD nWeaponDelay = pCustomDesc->m_nDelay;

	SetAnimationUpper(ZC_STATE_UPPER_SHOT);

	if(m_pVMesh->IsSelectWeaponGrenade()) {
		m_pVMesh->m_bGrenadeFire = true;
		m_pVMesh->m_GrenadeFireTime = GetGlobalTimeMS();
	}
}

bool ZMyCharacter::CheckWall(rvector& Pos)
{
	rvector vWPos;
	GetWeaponTypePos(weapon_dummy_muzzle_flash,&vWPos);

	rvector nPos = m_pVMesh->GetBipTypePosition(eq_parts_pos_info_Spine1);
	rvector nDir = vWPos - nPos;
	Normalize(nDir);

	RBSPPICKINFO bpi;

	memset(&bpi,0,sizeof(RBSPPICKINFO));

	if(ZGetGame()->GetWorld()->GetBsp()->Pick(nPos,nDir,&bpi)) {
		if(bpi.pInfo) {
			if(D3DXPlaneDotCoord(&(bpi.pInfo->plane),&vWPos) < 0) {
				Pos = bpi.PickPos - 2.f*nDir; 
				return true;
			}
		}
	}

	Pos = vWPos;

	return false;
}


void ZMyCharacter::OnShotRocket()
{
	ZItem* pSelectedItem = GetItems()->GetSelectedWeapon();

	if (pSelectedItem==NULL || pSelectedItem->GetBulletAMagazine() <= 0) {

		return;
	}

	MPOINT Cp = MPOINT(0, 0);
	RBSPPICKINFO bpi;

	memset(&bpi,0,sizeof(RBSPPICKINFO));

	ZCombatInterface* ci=ZApplication::GetGameInterface()->GetCombatInterface();

	if (ci)	{

		Cp = ci->GetCrosshairPoint();

		rvector pos,dir;
		RGetScreenLine(Cp.x,Cp.y,&pos,&dir);

		if(!ZGetGame()->GetWorld()->GetBsp()->Pick(pos,dir,&bpi)) {
			bpi.PickPos=pos+dir*10000.f;

		}
	}

	rvector vWeaponPos;

	CheckWall(vWeaponPos);

	rvector dir = bpi.PickPos - vWeaponPos;

	Normalize(dir);

	int sel_type = GetItems()->GetSelectedWeaponParts();

	ZPostShotSp(g_pGame->GetTime(),vWeaponPos,dir,ZC_WEAPON_SP_ROCKET,sel_type);
}

void ZMyCharacter::OnGadget_Hanging()
{
	switch(m_pVMesh->m_SelectWeaponMotionType)
	{	
		case eq_wd_katana : 
		case eq_wd_blade :
		case eq_wd_sword :
		case eq_ws_dagger:
		case eq_wd_dagger:
			break;
		default:
			return;
	};

	if(IsDie() || m_bWallJump || m_bGuard || m_bDrop || m_bTumble || m_bSkill ||
		m_bBlast || m_bBlastFall || m_bBlastDrop || m_bBlastStand || m_bBlastAirmove ) return;

	if(m_AniState_Lower==ZC_STATE_LOWER_JUMPATTACK) return;

	if(m_bWallJump2 && (g_pGame->GetTime()-m_fJump2Time)<.40f) return;

	m_bWallJump2=false;

	if(	!m_bWallHang )
	{
		if( GetDistToFloor()>100.f ) {
			m_fHangTime=g_pGame->GetTime();
			m_bWallHang=true;
			m_bHangSuccess=false;
		}
	} else {
		if(g_pGame->GetTime()-m_fHangTime>.4f && !m_bHangSuccess) {
			rvector pickorigin,pickto,dir;
			rvector front=m_Direction;
			front.z=0;
			Normalize(front);
			dir=front;
			pickorigin=m_Position+rvector(0,0,210);

			RBSPPICKINFO bpi;
			bool bPicked=ZGetGame()->GetWorld()->GetBsp()->Pick(pickorigin,dir,&bpi);

			if(bPicked && Magnitude(pickorigin-bpi.PickPos)<100.f) 
			{
				m_bHangSuccess=true;
				rplane plane=bpi.pInfo->plane;
				ZGetEffectManager()->AddLightFragment(bpi.PickPos,rvector(plane.a,plane.b,plane.c));

#ifndef _BIRDSOUND
				ZGetSoundEngine()->PlaySound("hangonwall", bpi.PickPos );
#endif
				SetLastThrower(MUID(0,0), 0.0f);
			}
			else
				m_bHangSuccess=false;
		}
	}
}

void ZMyCharacter::OnGadget_Snifer()
{
	m_bSniferMode = !m_bSniferMode;

	ZCombatInterface* ci=ZApplication::GetGameInterface()->GetCombatInterface();
	if (ci)
	{
		if (m_bSniferMode)
		{
			ci->OnGadget(MWT_SNIFER);
		}
		else
		{
			ci->OnGadgetOff();
		}
	}
}


void ZMyCharacter::ProcessGadget()
{
	if(m_bWallJump || m_bGuard || m_bStun || m_bShot || m_bSkill ||
		m_bSlash || m_bJumpSlash || m_bJumpSlashLanding ) return;

	if(GetItems()->GetSelectedWeapon()==NULL) return;

	if(IsDie() || m_bDrop || m_bBlast || m_bBlastDrop || m_bBlastStand) 
		return;

	if(m_AniState_Upper==ZC_STATE_UPPER_RELOAD || m_AniState_Upper==ZC_STATE_UPPER_LOAD) 
		return;

	bool bPressingSecondary = ZIsActionKeyPressed(ZACTION_USE_WEAPON2);
	bool bSecondary = bPressingSecondary && !bWasPressingSecondaryLastFrame;

	if(bSecondary && m_bLand) {

		MMatchWeaponType type = MWT_NONE;

		int sel_type = GetItems()->GetSelectedWeaponParts();
		ZItem* pSItem = GetItems()->GetSelectedWeapon();

		if(pSItem && pSItem->GetDesc())
			type = pSItem->GetDesc()->m_nWeaponType;
		switch(type) {
		case MWT_DAGGER :
		case MWT_DUAL_DAGGER :
			m_bSkill=true;
			m_fSkillTime = g_pGame->GetTime();
			m_bTumble = false;
			AddDelayedWork(g_pGame->GetTime()+0.18f,ZDW_DASH);
			break;
		case MWT_KATANA :
		case MWT_DOUBLE_KATANA :
			m_bSkill=true;
			m_fSkillTime = g_pGame->GetTime();
			m_bTumble = false;
			AddDelayedWork(g_pGame->GetTime()+0.18f,ZDW_UPPERCUT);
			break;
		}
	}

	/*
	if(m_bRButtonFirstPressed)
		AddDelayedWork(g_pGame->GetTime()+0.1f,ZDW_CHECKCHARGE);
	*/
	
	if (!bPressingSecondary)
	{
		m_bWallHang=false;
		m_bHangSuccess = false;
		bWasPressingSecondaryLastFrame = bPressingSecondary;
		return;
	}

	if (bSecondary)
	{
		ZItem* pSelectedItem = GetItems()->GetSelectedWeapon();
		if (pSelectedItem)
		{
			if(pSelectedItem->GetDesc()) {

				switch (pSelectedItem->GetDesc()->m_nWeaponType)
				{
				case MWT_SNIFER:
					{
						OnGadget_Snifer();
					}
					break;
				}

			}
		}
	}


	if (m_pVMesh)
	{
		if(!m_bLand)
			OnGadget_Hanging();
	}
	else
	{
		_ASSERT(0);
	}

	bWasPressingSecondaryLastFrame = bPressingSecondary;
}

void ZMyCharacter::ProcessGuard()
{
	if(IsDie() || m_bWallJump || m_bDrop || m_bWallJump2 || m_bTumble || 
		m_bBlast || m_bBlastFall || m_bBlastDrop || m_bBlastStand || m_bBlastAirmove ||
		m_bSlash || m_bJumpSlash || m_bJumpSlashLanding ) return;

	ZItem* pSItem = GetItems()->GetSelectedWeapon();
	if(!pSItem) return;

	if (pSItem->IsEmpty() || pSItem->GetDesc()==NULL) return;

	if(pSItem->GetDesc()->m_nType!=MMIT_MELEE) return;

	MMatchWeaponType type = pSItem->GetDesc()->m_nWeaponType;
	if(type!=MWT_KATANA && type!=MWT_DOUBLE_KATANA) return; 


	if(m_bGuard) {
		ReleaseLButtonQueue();
	}

#define GUARD_TIME	0.1f

	bool bBothPressed = m_bLButtonPressed && m_bRButtonPressed &&
		(m_bLButtonFirstPressed || m_bRButtonFirstPressed);

	float fTime = g_pGame->GetTime();
	bool bGuardTime = ((fTime-m_f1ShotTime)<=GUARD_TIME && m_bRButtonFirstPressed) ||
						((fTime-m_fSkillTime)<=GUARD_TIME && m_bLButtonFirstPressed) ||
						(m_bLButtonFirstPressed && m_bRButtonFirstPressed);

	if(!m_bGuard && m_bGuardKey) {
		bBothPressed = true;
		bGuardTime = true;
	}

	if(bBothPressed)
	{
		if(!m_bGuard && bGuardTime)
		{
			m_bSkill = false;
			m_bShot	= false;
			m_nShot	= 0;
			m_bGuard = true;
			m_bGuardStart = true;
			m_bWallHang = false;
			m_bJumpShot = false;
			m_fGuardStartTime = g_pGame->GetTime();

			if(m_bGuardKey) m_bGuardByKey = true;
			else			m_bGuardByKey = false;

			return;
		}
		ReleaseLButtonQueue();
	}
}

void ZMyCharacter::OnChangeWeapon(const char* WeaponModelName)
{
	ZCharacter::OnChangeWeapon(WeaponModelName);

	ReleaseLButtonQueue();
	m_fNextShotTime=0.f;
	m_fCAFactor = GetControllabilityFactor();
	m_fElapsedCAFactorTime = 0.0f;

	m_bSniferMode = false;

	ZCombatInterface* ci=ZApplication::GetGameInterface()->GetCombatInterface();
	if (ci)
	{
		ci->OnGadgetOff();
	}
}

void str_output(string& str,int i)
{

}

void str_line(string& str)
{

}

#define AddText(s) {str.Add(#s,false); str.Add(" :",false); str.Add(s);}

void ZMyCharacter::OutputDebugString_CharacterState()
{
	RDebugStr str;

	ZCharacter::OutputDebugString_CharacterState();

	AddText( m_fDeadTime );
	AddText( m_fNextShotTime );
	AddText( m_fWallJumpTime );
	AddText( m_bWallHang );
	AddText( m_bTumble );
	AddText( m_nTumbleDir );
	AddText( m_bMoving );
	AddText( m_bReleasedJump );
	AddText( m_bJumpQueued );
	AddText( m_bWallJumpQueued );
	AddText( m_fLastJumpPressedTime );

	AddText( m_fJump2Time );
	AddText( m_fHangTime );
	AddText( m_HangPos );
	AddText( m_bHangSuccess );
	AddText( m_nWallJump2Dir );
	AddText( m_fLastLButtonPressedTime );
	AddText( m_bEnterCharge );
	AddText( m_bJumpShot );
	AddText( m_bShot );
	AddText( m_bShotReturn );
	AddText( m_nShot );
	AddText( m_bSkill );

	AddText( m_bSplashShot );
	AddText( m_fSplashShotTime );
	AddText( m_fLastShotTime );
	AddText( m_bGuard );
	AddText( m_nGuardBlock );
	AddText( m_bGuardBlock_ret );
	AddText( m_bGuardStart );
	AddText( m_bGuardCancel );
	AddText( m_bDrop );
	AddText( m_fDropTime );

	str.PrintLog();
}

void ZMyCharacter::ProcessShot()
{
	if (ZApplication::GetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_PREPARE) return;
	if (m_bInitialized==false) return;
	if (MEvent::GetRButtonState()) return;	// 오른버튼 누른상태서 왼버튼은 ProcessGadget()이 처리

	/*
	m_bLButtonPressed = MEvent::GetLButtonState() || MEvent::IsKeyDown(VK_CONTROL);

	if(m_bLButtonPressed) {
		if(m_bLButtonReleased)
			m_fLastLButtonPressedTime=g_pGame->GetTime();
		m_bLButtonReleased=false;
	} else {
		m_bLButtonReleased=true;
		m_bEnterCharge=false;
	}
	*/

	if(!m_bLButtonPressed) m_bEnterCharge = false;	// 중간에 떼면 기모으기로 넘어가지 않는다

	if(m_bLButtonFirstPressed && (g_pGame->GetTime() - m_fLastLButtonPressedTime < SHOT_QUEUE_TIME))
		m_bLButtonQueued = true;

	if (GetItems()->GetSelectedWeapon()==NULL ||
		GetItems()->GetSelectedWeapon()->IsEmpty()) return;

	if(GetItems()->GetSelectedWeapon()->GetDesc()->m_nType==MMIT_MELEE)
	{
		// 공중 칼질의 경우 예약 적용 안받기..
		bool bJumpAttack = false;
		if( m_AniState_Lower==ZC_STATE_LOWER_JUMPATTACK ) {
			bJumpAttack = true;
		} 
		else if( (m_AniState_Lower==ZC_STATE_LOWER_JUMP_UP) || (m_AniState_Lower==ZC_STATE_LOWER_JUMP_DOWN) ) {
			if(m_AniState_Upper == ZC_STATE_UPPER_SHOT) {
				bJumpAttack = true;
			}
		}

		if(bJumpAttack) {//점프하면서 공격하는경우
			//bLButtonQueued = false;
			ReleaseLButtonQueue();
			if(!m_bLButtonPressed)
				return;
		}
		else {
			if(!m_bLButtonQueued) 
				return;
		}
	}
	else
	{
		if (ZApplication::GetGame()->GetMatch()->IsRuleGladiator() && !IsAdmin()) return;
		if(!m_bLButtonPressed) return;
	}

	if(GetItems()->GetSelectedWeapon()->GetDesc()->m_nType!=MMIT_MELEE)//희철씨 칼인 경우는 놔두고..
		if(m_bWallJump) {
			if(m_nWallJumpDir==1)//앞벽달리기시~
				return;
		}

/*
	if(m_bWallJump2) {
		if( (m_nWallJump2Dir==7) || (m_nWallJump2Dir==6) )//앞벽달리기떨어질때~
			return;
	}
*/
//	if(GetItems()->GetSelectedWeapon()->IsEmpty() == true) return;		// 총알부족

	if(m_bWallHang || m_bStun ) return;		// 벽에 매달리면 쏠수없다.

	if(IsDie() || m_bDrop || m_bBlast || m_bBlastFall || m_bBlastDrop || m_bBlastStand || m_bSpMotion) 
		return;		// 죽거나 누워있어도 쏠수없다.

	if(m_AniState_Upper==ZC_STATE_UPPER_RELOAD || m_AniState_Upper==ZC_STATE_UPPER_LOAD) 
		return;		// 리로드중에 쏠수없다

	int nParts = (int)GetItems()->GetSelectedWeaponParts();

	/*
		 if(nParts==MMCIP_MELEE)		nParts = ZCWT_MELEE;
	else if(nParts==MMCIP_PRIMARY)		nParts = ZCWT_PRIMARY;
	else if(nParts==MMCIP_SECONDARY)	nParts = ZCWT_SECONDARY;
	else if(nParts==MMCIP_CUSTOM1)		nParts = ZCWT_CUSTOM1;
	else if(nParts==MMCIP_CUSTOM2)		nParts = ZCWT_CUSTOM2;
	else 
		return;

	if(nParts<0 || nParts > ZCWT_END-1)
		return;
	*/
	if(nParts<0 || nParts > MMCIP_END-1) return;

	if(GetItems()->GetSelectedWeapon()->GetDesc()->m_nType!=MMIT_MELEE)  //melee 계열은 에니메이션 속도로 제어
		if( m_fNextShotTimeType[nParts] > g_pGame->GetTime() )
			return;

	ZItem* pSelectedItem = GetItems()->GetSelectedWeapon();

	if ((pSelectedItem==NULL) || (pSelectedItem->GetDesc() == NULL)) 
		return;

	MMatchItemDesc* pRangeDesc = pSelectedItem->GetDesc();
	DWORD nWeaponDelay = pRangeDesc->m_nDelay;

	// 단도의 딜레이는 끝난 시간을 기준으로 한다. 애니메이션이 끝날때 다시 세팅해준다.

	m_fNextShotTimeType[nParts] = g_pGame->GetTime()+(float)(nWeaponDelay)*0.001f;

	if(GetItems()->GetSelectedWeapon()->GetBulletAMagazine() <= 0 ) 
	{
		if( m_pVMesh->m_SelectWeaponMotionType !=  eq_wd_grenade && 
			m_pVMesh->m_SelectWeaponMotionType != eq_wd_item && 
			m_pVMesh->m_SelectWeaponMotionType != eq_wd_katana && 
			m_pVMesh->m_SelectWeaponMotionType != eq_ws_dagger && 
			m_pVMesh->m_SelectWeaponMotionType != eq_wd_dagger && 
			m_pVMesh->m_SelectWeaponMotionType != eq_wd_sword &&
			m_pVMesh->m_SelectWeaponMotionType != eq_wd_blade )	
		{
			if( GetItems()->GetSelectedWeapon()->GetBullet() <= 0 ) {
				ZGetSoundEngine()->PlaySEDryFire( GetItems()->GetSelectedWeapon()->GetDesc(), 0, 0, 0, true );
			}
			else {
				m_bSpMotion = false;// 특수모션은 취소.. 
				ZPostReload();	
			}
		}
	}

	switch( m_pVMesh->m_SelectWeaponMotionType ) {
		case eq_wd_grenade :
			OnShotCustom();
			break;
		case eq_wd_item :
			if (m_bLand) OnShotItem();
			break;
		case eq_wd_rlauncher :
			OnShotRocket();
			break;
		case eq_wd_katana :
		case eq_ws_dagger :
		case eq_wd_dagger :
		case eq_wd_sword :
		case eq_wd_blade :
			OnShotMelee();
			break;
		default:
			OnShotRange();
	}

	UpdateStylishShoted();
}

void ZMyCharacter::OnRecoil(int nControlability)
{
	float fCameraDeltaX = 0.0f;
	float fCameraDeltaZ = 0.0f;

#define RECOIL_ANGLE	0.004106657f
	fCameraDeltaX = RECOIL_ANGLE * nControlability;
//	fCameraDeltaZ = (((rand() % 200) / 100.0f)-1.0) * RECOIL_ANGLE * nControlability;

	ZCamera* pcamera = ZApplication::GetGameInterface()->GetCamera();

	// 반동
	pcamera->SetMaxRecoilAngleX(fCameraDeltaX*2);
	pcamera->RecoilAngle(fCameraDeltaX, fCameraDeltaZ);

	pcamera->m_fAngleX=max(CAMERA_ANGLEX_MIN,pcamera->m_fAngleX);
	pcamera->m_fAngleX=min(CAMERA_ANGLEX_MAX,pcamera->m_fAngleX);
}
/*
bool ZMyCharacter::CheckStamina(int v)
{
	ZCharacterStatus* pCS = GetStatus();

	if(pCS) {
		if( pCS->fStamina >= v) {
			return true;
		}
	}

	return false;
}

bool ZMyCharacter::UsingStamina(int mode)
{
	ZCharacterStatus* pCS = GetStatus();

	if(pCS) {

		float fSubStamina = 0.f;

		if(mode==ZUS_Tumble) {
			fSubStamina = 20.f;
		}
		else if(mode==ZUS_Jump) {
			fSubStamina = 5.f;
		}

		if( pCS->fStamina >= fSubStamina) {
			
			pCS->fStamina = max( pCS->fStamina - fSubStamina , 0.f );

			return true;
		}
	}

	return false;
}

void ZMyCharacter::UpdateStamina(float fTime)
{
	if( m_bTumble )
		return;

	ZCharacterStatus* pCS = GetStatus();

	if(pCS) {

		float fAddStamina = 15.f;//초당회복량 - 필요하면 아이템으로 빼기~
		float fAddStaminaMax  = 100.f;

		if( pCS->fStamina < fAddStaminaMax) {

			float fAdd = 0.f;

			if(m_bMoving) {
				fAdd = fAddStamina * fTime * 0.5f;//50% 회복
			}
			else {
				fAdd = fAddStamina * fTime;
			}

			pCS->fStamina = min( pCS->fStamina + fAdd , fAddStaminaMax );
		}
	}
}
*/
void ZMyCharacter::UpdateAnimation()
{
	if (m_bInitialized==false) return;
/*
	if(m_bMoveLimit) {
		SetAnimationLower(ZC_STATE_LOWER_IDLE1);
		return;
	}
*/
	bool bTaunt = false;

	if(m_bStun)
	{
		switch(m_nStunType) {
		case 0 : SetAnimationLower(ZC_STATE_DAMAGE); break;
		case 1 : SetAnimationLower(ZC_STATE_DAMAGE2); break;
		case 2 : SetAnimationLower(ZC_STATE_DAMAGE_DOWN); break;
		case 3 : SetAnimationLower(ZC_STATE_DAMAGE_BLOCKED); break;
		case 4 : SetAnimationLower(ZC_STATE_DAMAGE_LIGHTNING); break;
		case 5 : SetAnimationLower(ZC_STATE_DAMAGE_STUN); break;
		}		
	}else
	if(m_bBlastAirmove)
	{
		SetAnimationLower(ZC_STATE_LOWER_BLAST_AIRMOVE);
	}else
	if(m_bBlastStand)
	{
		SetAnimationLower(ZC_STATE_LOWER_BLAST_STAND);
	}else
	if(m_bBlastFall)
	{
			 if(m_nBlastType==0) SetAnimationLower(ZC_STATE_LOWER_BLAST_FALL);
		else if(m_nBlastType==1) SetAnimationLower(ZC_STATE_LOWER_BLAST_DROP_DAGGER);
	}else
	if(m_bBlastDrop)
	{
			 if(m_nBlastType==0) SetAnimationLower(ZC_STATE_LOWER_BLAST_DROP);
		else if(m_nBlastType==1) SetAnimationLower(ZC_STATE_LOWER_BLAST_DROP_DAGGER);

	}else
	if(m_bBlast)
	{
			 if(m_nBlastType==0) SetAnimationLower(ZC_STATE_LOWER_BLAST);
		else if(m_nBlastType==1) SetAnimationLower(ZC_STATE_LOWER_BLAST_DAGGER);

	}else
	if(m_bSkill)
	{
		SetAnimationLower(ZC_STATE_LOWER_UPPERCUT);
	}else
	if(m_bJumpShot)
	{
/*
	// 나중에 진호씨가 모션 완성하면 풀어주자..

		if(m_nJumpShotType==0)
			SetAnimationLower(ZC_STATE_LOWER_JUMPATTACK);
		else 
			SetAnimationLower(ZC_STATE_LOWER_RUN_WALL_CANCEL);
*/ 
		SetAnimationLower(ZC_STATE_LOWER_JUMPATTACK);

	}else
	if(m_bGuard)
	{
		if(m_bJumpUp) SetAnimationLower(ZC_STATE_LOWER_JUMP_UP);
		else
		if(m_bJumpDown) SetAnimationLower(ZC_STATE_LOWER_JUMP_DOWN);
		else
			if(m_bMoving)
			{
				if(m_bBackMoving) SetAnimationLower(ZC_STATE_LOWER_RUN_BACK);
				else 
					SetAnimationLower(ZC_STATE_LOWER_RUN_FORWARD);
			}
			else
				SetAnimationLower(ZC_STATE_LOWER_IDLE1);

		if(m_bGuardStart)
			SetAnimationUpper(ZC_STATE_UPPER_GUARD_START);
		else
		if(m_bGuardCancel)
			SetAnimationUpper(ZC_STATE_UPPER_GUARD_CANCEL);
		else
		if(m_nGuardBlock)
		{
			switch(m_nGuardBlock) {
			case 1 : SetAnimationUpper(ZC_STATE_UPPER_GUARD_BLOCK1);break;
			case 2 : SetAnimationUpper(ZC_STATE_UPPER_GUARD_BLOCK2);break;
			}
		}else
		if(m_bGuardBlock_ret)
			SetAnimationUpper(ZC_STATE_UPPER_GUARD_BLOCK1_RET);
		else
			SetAnimationUpper(ZC_STATE_UPPER_GUARD_IDLE);
	}else
	if(m_bShotReturn)
	{
		switch(m_nShot) {
		case 1 : SetAnimationLower(ZC_STATE_LOWER_ATTACK1_RET); break;
		case 2 : SetAnimationLower(ZC_STATE_LOWER_ATTACK2_RET); break;
		case 3 : SetAnimationLower(ZC_STATE_LOWER_ATTACK3_RET); break;
		case 4 : SetAnimationLower(ZC_STATE_LOWER_ATTACK4_RET); break;
		}
	}else
	if(m_bShot)
	{
		switch(m_nShot) {
		case 1 : SetAnimationLower(ZC_STATE_LOWER_ATTACK1); break;
		case 2 : SetAnimationLower(ZC_STATE_LOWER_ATTACK2); break;
		case 3 : SetAnimationLower(ZC_STATE_LOWER_ATTACK3); break;
		case 4 : 
			SetAnimationLower(ZC_STATE_LOWER_ATTACK4); 
//			ZCamera* pCamera = ZApplication::GetGameInterface()->GetCamera();
//			pCamera->Shock(4.0f, 50.0f, rvector(0.0f, 0.0f, -1.0f));	// 카메라 반동..-_-;
			break;
		case 5 : SetAnimationLower(ZC_STATE_LOWER_ATTACK5); break;
		}
	}else
	if(m_bWallJump2)
	{
		switch(m_nWallJump2Dir) {
		case 0 : SetAnimationLower(ZC_STATE_LOWER_JUMP_WALL_FORWARD); break;
		case 1 : SetAnimationLower(ZC_STATE_LOWER_JUMP_WALL_BACK); break;
		case 2 : SetAnimationLower(ZC_STATE_LOWER_JUMP_WALL_LEFT); break;
		case 3 : SetAnimationLower(ZC_STATE_LOWER_JUMP_WALL_RIGHT); break;
		case 4 : SetAnimationLower(ZC_STATE_LOWER_RUN_WALL_LEFT_DOWN); break;
		case 5 : SetAnimationLower(ZC_STATE_LOWER_RUN_WALL_RIGHT_DOWN); break;
		case 6 : SetAnimationLower(ZC_STATE_LOWER_RUN_WALL_DOWN_FORWARD); break;
		case 7 : SetAnimationLower(ZC_STATE_LOWER_RUN_WALL_DOWN); break;
//		case 4 : SetAnimationLower(ZC_STATE_LOWER_RUN_WALL);break;
		}
	}else
	if(m_bWallHang)
	{
		SetAnimationLower(ZC_STATE_LOWER_BIND);
	}else
	if(m_bTumble)
	{
		switch(m_nTumbleDir)
		{
		case 0 :SetAnimationLower(ZC_STATE_LOWER_TUMBLE_FORWARD);break;
		case 1 :SetAnimationLower(ZC_STATE_LOWER_TUMBLE_BACK);break;
		case 2 :SetAnimationLower(ZC_STATE_LOWER_TUMBLE_RIGHT);break;
		case 3 :SetAnimationLower(ZC_STATE_LOWER_TUMBLE_LEFT);break;
		}
	}else
	if(m_bWallJump)
	{
		switch(m_nWallJumpDir)
		{
			case 0 :SetAnimationLower(ZC_STATE_LOWER_RUN_WALL_LEFT);break;
			case 1 :SetAnimationLower(ZC_STATE_LOWER_RUN_WALL);break;
			case 2 :SetAnimationLower(ZC_STATE_LOWER_RUN_WALL_RIGHT);break;
		}
	}else 
	if(m_bJumpSlash) {
		SetAnimationLower(ZC_STATE_JUMP_SLASH1);
	}else 
	if(m_bJumpSlashLanding) {
		SetAnimationLower(ZC_STATE_JUMP_SLASH2);
	}else
	if(m_bSlash) {
		SetAnimationLower(ZC_STATE_SLASH);
	}else
	if(m_bJumpUp)
	{
		SetAnimationLower(ZC_STATE_LOWER_JUMP_UP);
	}else
	if(m_bJumpDown)
	{
		SetAnimationLower(ZC_STATE_LOWER_JUMP_DOWN);
	}else 
	if(m_bCharging) {
		SetAnimationLower(ZC_STATE_CHARGE);
	}else
	if(m_bMoving){

		if(m_bBackMoving)
			SetAnimationLower(ZC_STATE_LOWER_RUN_BACK);
		else {
			SetAnimationLower(ZC_STATE_LOWER_RUN_FORWARD);
		}
	}else 
	if(m_bSpMotion) {

		SetAnimationLower(m_SpMotion);

		bTaunt = true;
	} else
		SetAnimationLower(ZC_STATE_LOWER_IDLE1);

	if(m_bSpMotion) {// taunt 면서 다른 상태로 바뀌었다면 취소~
		if(!bTaunt)
			m_bSpMotion = false;
	}

}

void ZMyCharacter::WallJump2()
{
	m_bWallJump2=true;
	m_bJumpQueued=false;
	m_bWallJump=false;
	m_bPlayDone=false;

	rvector jump2=-rvector(m_Direction.x,m_Direction.y,0);
	Normalize(jump2);

	if(m_nWallJumpDir==1)
	{
		if(ZIsActionKeyPressed(ZACTION_FORWARD))
		{
			m_nWallJump2Dir=6;
			SetVelocity(jump2*50.f);
		}
		else
		{
			m_nWallJump2Dir=7;
			AddVelocity(jump2*300.f);
		}
		AddVelocity(rvector(0,0,1400));

	}else
	{
		float fSecondJumpSpeed = WALL_JUMP2_SIDE_VELOCITY;

		rvector right;
		CrossProduct(&right,m_Direction,rvector(0,0,1));
		jump2= (m_nWallJumpDir==0) ? -right : right;
		if(m_nWallJumpDir==0)
			m_nWallJump2Dir=4;
		else
			m_nWallJump2Dir=5;

		AddVelocity(fSecondJumpSpeed*jump2);
		AddVelocity(rvector(0,0,1300));
	}

	m_fJump2Time=g_pGame->GetTime();
}

void ZMyCharacter::UpdateLimit()
{
	// 우선선택된 무기들만 제한에 영향을 준다.. 반지나 의류등도 필요할때 추가..

	MMatchItemDesc* pDesc = GetSelectItemDesc();

	m_bLimitJump	= false;
	m_bLimitTumble	= false;
	m_bLimitWall	= false;

	if(pDesc) {
	
		if(pDesc->m_nLimitJump)			m_bLimitJump	= true;
		if(pDesc->m_nLimitTumble)		m_bLimitTumble	= true;
		if(pDesc->m_nLimitWall)			m_bLimitWall	= true;
	}

	// cold 데미지중이면 모든 특수동작 봉쇄
	if(IsActiveModule(ZMID_COLDDAMAGE)) {
//		m_bLimitJump	= true;
		m_bLimitTumble	= true;
		m_bLimitWall	= true;
	}
}

void ZMyCharacter::UpdateButtonState()
{
//	bool bLButtonPressed = MEvent::GetLButtonState() || MEvent::IsKeyDown(VK_CONTROL);
//	modified by 정동섭 @ 2006/3/16 : 컨트롤키로 샷 안되게 함
//	혹시 좌클릭으로만 가능하고 키보드의 샷키로는 불가능해야 하는 일이 있나요? -_-a

	bool bLButtonPressed = ZIsActionKeyPressed(ZACTION_USE_WEAPON);

	// 안눌러져있다가 눌러진상태 검사
	m_bLButtonFirstPressed = bLButtonPressed && !m_bLButtonPressed;
	if(m_bLButtonFirstPressed) 
		m_fLastLButtonPressedTime=g_pGame->GetTime();

	m_bLButtonPressed = bLButtonPressed;


	bool bRButtonPressed = MEvent::GetRButtonState();

	// 눌려져있다가 떼진상태 검사
	m_bRButtonFirstReleased = (m_bRButtonPressed && !bRButtonPressed);	

	// 안눌러져있다가 눌러진상태 검사
	m_bRButtonFirstPressed = bRButtonPressed && !m_bRButtonPressed;
	if(m_bRButtonFirstPressed) 
		m_fLastRButtonPressedTime = g_pGame->GetTime();

	m_bRButtonPressed = bRButtonPressed;

	// 칼질 예약 타이밍이 지나면 풀어준다
	if(m_bLButtonQueued && (g_pGame->GetTime()-m_fLastLButtonPressedTime>SHOT_QUEUE_TIME)) {
		m_bLButtonQueued = false;
	}
}

void ZMyCharacter::OnUpdate(float fDelta)
{
	if (m_bInitialized==false) return;

	_BP("ZMyCharacter::Update");

	if(g_pGame->IsReplay()) {
		ZCharacter::OnUpdate(fDelta);
		return;
	}

	CameraDir = m_TargetDir;

	if(m_bReserveDashAttacked) {
		if( g_pGame->GetTime() > m_fReserveDashAttackedTime ) {
			OnDashAttacked(m_vReserveDashAttackedDir);
			m_bReserveDashAttacked = false;
		}
	}

	UpdateLimit();

	if((m_bWallHang && m_bHangSuccess) || m_bShot) 
		SetVelocity(0,0,0);
	
	UpdateVelocity(fDelta);

	fDelta=min(fDelta,1.f);

	if(IsDie() && ((m_bLand && m_bPlayDone) )) {
		SetVelocity(0,0,0);
	}

	m_bMoving = Magnitude(rvector(GetVelocity().x,GetVelocity().y,0))>.1f;

	const int MAX_MOVE_FRAME = 150;
	static float fAccmulatedDelta = 0.f;

	fAccmulatedDelta+=fDelta;

	if(fAccmulatedDelta>(1.f/(float)MAX_MOVE_FRAME))
	{
		m_pModule_Movable->Update(fAccmulatedDelta);

		if(Magnitude(m_pModule_Movable->GetLastMove())<0.01f)
			SetVelocity(0,0,0);

		if (GetDistToFloor() > 0.1f || GetVelocity().z > 0.1f)
		{
			m_pModule_Movable->UpdateGravity(GetGravityConst()*fAccmulatedDelta);
			m_bLand=false;
		}

		fAccmulatedDelta = 0;
	}

	UpdateHeight(fDelta);

	rvector diff=fDelta*GetVelocity();

	bool bUp = (diff.z>0.01f);
	bool bDownward= (diff.z<0.01f);

	if ((m_Position.z > DIE_CRITICAL_LINE) &&
		(GetDistToFloor()<0 || (bDownward && m_pModule_Movable->GetLastMove().z>=0)))
	{
		if(!m_bWallJump)
		{
			if(GetVelocity().z<1.f && GetDistToFloor()<1.f)
			{
				m_bLand=true;
				m_bWallJump=false;
				m_bJumpDown=false;
				m_bJumpUp=false;
				m_bWallJump2=false;
				m_bBlastAirmove=false;
				m_bWallHang=false;
				
				if(GetVelocity().z<0)
					SetVelocity(GetVelocity().x,GetVelocity().y,0);

				if (GetLastThrowClearTime() < g_pGame->GetTime())
					SetLastThrower(MUID(0,0), 0.0f);
			}
		}
	} 

	UpdateCAFactor(fDelta);

	// 계단 같은곳에서 스르륵 올라오도록..
	if(GetDistToFloor() < 0 && !IsDie())
	{
		float fAdjust=400.f*fDelta;	// 초당 이만큼
		rvector diff=rvector(0,0,min(-GetDistToFloor(),fAdjust));
		Move(diff);
	}

	if(IsMoveAnimation())		// 애니메이션에 움직임이 포함된넘은 움직여줘야한다.
	{
		rvector origdiff=fDelta*GetVelocity();

		rvector diff = m_AnimationPositionDiff;
		diff.z+=origdiff.z;


//		rvector diff=m_RealPositionBefore-m_RealPosition;	// mesh 의 위치 변화로 움직임판정

		if(GetDistToFloor()<0 && diff.z<0) diff.z=-GetDistToFloor();

		Move(diff);
	}

	//if (IsKnockBack())
	//	OnKnockBack();

#ifndef _PUBLISH
	// 방향 보정
	if(!MEvent::IsKeyDown(VK_MENU))
#endif
	if (!m_bDie)
	{
		if( !m_bSkill&&!m_bWallJump && !m_bWallJump2 && !m_bWallHang && 
			!m_bTumble && !m_bBlast && !m_bBlastStand && !m_bBlastDrop )
		{
			if(!_isnan(RCameraDirection.x) && !_isnan(RCameraDirection.y) && !_isnan(RCameraDirection.z) )
				SetTargetDir( rvector(RCameraDirection.x,RCameraDirection.y,RCameraDirection.z) );
		}
	}


//	UpdateHeight(fDelta);

	// 벽점푸 풀리는거 하드코딩 -_-;

	float fWallJumpTime = (m_nWallJumpDir==1) ? 1.5f : 2.3f;
	float fSecondJumpTime = (m_nWallJumpDir==1) ? 0.95f : 2.1f;

	if(m_fWallJumpTime+fWallJumpTime<g_pGame->GetTime() && m_bWallJump)
//	if(m_bWallJump && m_pVMesh->m_isPlayDone[ani_mode_lower])
	{
		m_bWallJump=false;
		m_bLand=true;
		m_bJumpDown=false;
		m_bJumpUp=false;

		SetVelocity(GetVelocity().x,GetVelocity().y,0);
	}

	// 무엇인가에 걸려서 벽점프의 마무리 점프를 해야 할 상황인지 판단.
	if(m_bWallJump)
	{
		bool bEndWallJump2=m_fWallJumpTime+fSecondJumpTime<g_pGame->GetTime() && !m_bWallJump2;

		if(m_fWallJumpTime+0.3f<g_pGame->GetTime())	// 0.3초가 지나야 한다.
		{
			rvector dir2d=rvector(m_Direction.x,m_Direction.y,0);
			Normalize(dir2d);

			if(m_nWallJumpDir==1)									// 위방향 벽점프
			{
				// 발디딜곳이.. 있어야 하고 ( 앞방향으로 pick 해본다 )
				RBSPPICKINFO bpi;
				bool bPicked=ZGetGame()->GetWorld()->GetBsp()->Pick(m_Position+rvector(0,0,40),dir2d,&bpi);
				if(!bPicked || Magnitude(bpi.PickPos-m_Position)>100)
					bEndWallJump2|=true;

				// 대략 머리위 30센티미터쯤까지는 걸리는게 없어야 한다.
				rvector targetpos=m_Position+rvector(0,0,160);
				bool bAdjusted=ZGetGame()->GetWorld()->GetBsp()->CheckWall(m_Position+rvector(0,0,130),targetpos,CHARACTER_RADIUS-1,50);
				if(bAdjusted)
					bEndWallJump2|=true;
			}
			else
			{
				rvector right;
				CrossProduct(&right,rvector(0,0,1),dir2d);

				rvector dir = (m_nWallJumpDir==0) ? -right : right;		// 왼쪽 오른쪽

				// 발 디딜곳이 있어야 하고..
				RBSPPICKINFO bpi;
				bool bPicked=ZGetGame()->GetWorld()->GetBsp()->Pick(m_Position,dir,&bpi);
				if(!bPicked || Magnitude(bpi.PickPos-m_Position)>100)
					bEndWallJump2|=true;

				// 대략 1미터쯤까지는 걸리는게 없어야 한다.
				rvector targetpos=m_Position+rvector(0,0,100)+100.f*dir2d;
				bool bAdjusted=ZGetGame()->GetWorld()->GetBsp()->CheckWall(m_Position+rvector(0,0,100),targetpos,CHARACTER_RADIUS-1,50);
				if(bAdjusted)
					bEndWallJump2|=true;
			}
		}

		if(bEndWallJump2)
			WallJump2();
	}

	int sel_type = GetItems()->GetSelectedWeaponParts();
/*
	ZItem* pSItem = GetItems()->GetSelectedWeapon();

	if(pSItem) {
		pSItem->GetItemType()
	}
*/

	// 첫번째 칼질은 딜레이가 있다
	if(m_bShot && m_nShot==1 && !m_b1ShotSended && g_pGame->GetTime()-m_f1ShotTime>GUARD_TIME) {
		m_b1ShotSended=true;
//		mlog("zpost_melee 첫번째칼질!\n");
		if (g_Rules.IsVanillaMode())
		{
			ZPostShotMelee(g_pGame->GetTime(), m_Position, 1);
			AddDelayedWork(g_pGame->GetTime() + 0.2f, ZDW_RECOIL);
		}
		else
		{
			AddDelayedWork(g_pGame->GetTime() + 0.1, ZDW_RG_SLASH, (void *)1);
		}
	}

	// ZDelayedWork 으로 대체
	/*
	// TODO : 미리 패킷은 발사하고 딜레이를 주자.
	// 어퍼켓 딜레이
	if(m_bSkill && !m_bSkillSended && g_pGame->GetTime()-m_fSkillTime>.16f)	
	{
		m_bSkillSended=true;

		// 가진무기에따라서..
//		RWeaponType type = m_pVMesh->GetSelectWeaponType();
		MMatchWeaponType type = MWT_NONE;

		ZItem* pSItem = GetItems()->GetSelectedWeapon();

		if(pSItem && pSItem->GetDesc())
			type = pSItem->GetDesc()->m_nWeaponType;

		if(type == MWT_KATANA) {
			ZPostSkill( g_pGame->GetTime(), ZC_SKILL_UPPERCUT , sel_type );
		}
		else if(type == MWT_DAGGER) {//한손대거..
			ZPostSkill( g_pGame->GetTime(), ZC_SKILL_DASH , sel_type );
		}
		else if(type == MWT_DUAL_DAGGER) {//양손대거..

		}
	}
	*/

	// 일정시간이 지나 stun (type5)가 풀림
	if(m_bStun && m_nStunType==ZST_LOOP && g_pGame->GetTime()>m_fStunEndTime) {
		m_bStun = false;
	}

	// 일정시간 지나면 guard 도 풀림
	if(
#ifdef _DEBUG
		!m_bGuardTest && 
#endif		
		m_bGuard && g_pGame->GetTime()-m_fGuardStartTime>GUARD_DURATION) {
		m_nGuardBlock=0;
		m_bGuardStart=false;
		m_bGuardCancel=true;
		m_bGuardKey = false;
		m_bGuardByKey = false;
	}

	// 중간에 떼거나 무기가 바뀌면 기모으기가 풀린다
	if(!m_bLButtonPressed || GetItems()->GetSelectedWeaponParts()!=MMCIP_MELEE) 
		m_bCharging = false;

	// 마지막 스플래시 칼질 딜레이
	if(m_bSplashShot && g_pGame->GetTime()-m_fSplashShotTime>.3f)
	{
		m_bSplashShot=false;
		ZPostSkill( g_pGame->GetTime(), ZC_SKILL_SPLASHSHOT , sel_type);
	}

	AniFrameInfo* pAniLow = m_pVMesh->GetFrameInfo(ani_mode_lower);

	if(pAniLow && pAniLow->m_isPlayDone) {
		if(m_bSpMotion) {
			if(m_AniState_Lower==m_SpMotion)
				m_bSpMotion = false;
		}
	}
/*	
	if(m_pVMesh->m_isPlayDone[ani_mode_lower]) {
		if(m_bSpMotion) {
			if(m_AniState_Lower==m_SpMotion)
				m_bSpMotion = false;
		}
	}
*/
	bool bContinueShot = false;

	// 상체 애니메이션이 끝난경우
	if(m_bPlayDone_upper) {
		if(m_AniState_Upper==ZC_STATE_UPPER_SHOT) 
		{
			// 단도의 딜레이는 끝난 시간을 기준으로 한다
			int nParts = (int)GetItems()->GetSelectedWeaponParts();
			if(nParts==MMCIP_MELEE)
			{
//				nParts = ZCWT_MELEE;

				ZItem* pSelectedItem = GetItems()->GetSelectedWeapon();
				_ASSERT(pSelectedItem && pSelectedItem->GetDesc());

				if(pSelectedItem) {
					MMatchItemDesc* pRangeDesc = pSelectedItem->GetDesc();
					DWORD nWeaponDelay = pRangeDesc->m_nDelay;

					m_fNextShotTimeType[nParts] = g_pGame->GetTime()+(float)(nWeaponDelay)*0.001f;
				}

				// 단검 기모으기
				if(m_bEnterCharge)
					EnterCharge();
			}
			else if ( (nParts==MMCIP_CUSTOM1) || (nParts==MMCIP_CUSTOM2) ) {	
				// 사용해서 없어지는 아이템이면 다쓴뒤 무기체인지
				ZItem* pItem = GetItems()->GetSelectedWeapon();
				if (pItem && pItem->GetBulletAMagazine() <= 0)
					ZGetGameInterface()->ChangeWeapon(ZCWT_PREV);				
			}
		}else
		if(m_AniState_Upper==ZC_STATE_UPPER_RELOAD)
			g_pGame->OnReloadComplete(this);
		else
		if(m_bGuard) {

			if(m_bGuardBlock_ret) {

				m_bGuardBlock_ret=false;
			} 
			else if(m_nGuardBlock)	{

				if(m_nGuardBlock<2 && g_pGame->GetTime()-m_fLastShotTime<0.2f) {
					m_nGuardBlock++;
					m_bPlayDone_upper=false;
				}
				else {

					if(m_nGuardBlock==1)
						m_bGuardBlock_ret=true;

					m_bPlayDone_upper=false;
					m_nGuardBlock=0;
				}
			}

			m_bGuardStart=false;

			if(m_bGuardCancel) {

				m_bGuardCancel=false;
				m_bGuard=false;

			}

		}

		// 연속으로 발사되어서 현재 상태가 유지 되어야하는 경우는 놔둬야한다~
/*
		if( m_pVMesh->m_pAniSet[ani_mode_upper] )
			if( m_pVMesh->m_pAniSet[ani_mode_upper]->m_ani_loop_type != RAniLoopType_Loop )
				SetAnimationUpper(ZC_STATE_UPPER_NONE);
		else 
*/
/*
		if(m_pVMesh) {

			if( m_pVMesh->GetSelectWeaponType() == z_wd_machinegun) {

				int nParts = (int)GetItems()->GetSelectedWeaponParts();

				if(nParts==MMCIP_MELEE)		nParts = ZCWT_MELEE;
				else if(nParts==MMCIP_PRIMARY)		nParts = ZCWT_PRIMARY;
				else if(nParts==MMCIP_SECONDARY)	nParts = ZCWT_SECONDARY;
				else if(nParts==MMCIP_CUSTOM1)		nParts = ZCWT_CUSTOM1;
				else if(nParts==MMCIP_CUSTOM2)		nParts = ZCWT_CUSTOM2;
				else {// 설마~
					//					return;
				}

				if( m_fNextShotTimeType[nParts]+0.3f > g_pGame->GetTime() ) {//다음 공격 시간보다 작다면 지금 공격중이다~
					bContinueShot = true; 
				}
			}
		}

		if(bContinueShot)
			SetAnimationUpper(ZC_STATE_UPPER_SHOT);
		else
			SetAnimationUpper(ZC_STATE_UPPER_NONE);
*/
			SetAnimationUpper(ZC_STATE_UPPER_NONE);
	}

	AniFrameInfo* pAniUp = m_pVMesh->GetFrameInfo(ani_mode_upper);

	if(pAniUp ) {
		if( pAniUp->m_pAniSet==NULL) {
			SetAnimationUpper( ZC_STATE_UPPER_NONE );
		}
	}
/*
	if( m_pVMesh->m_pAniSet[ ani_mode_upper ] == NULL ) {
		SetAnimationUpper( ZC_STATE_UPPER_NONE );
	}
*/
	MMatchWeaponType wtype = MWT_NONE;

	ZItem* pSItem = GetItems()->GetSelectedWeapon();

	if( pSItem && pSItem->GetDesc() ) {
		wtype = pSItem->GetDesc()->m_nWeaponType;
	}


	// 하체 애니메이션이 끝난경우

	if(m_bPlayDone)
	{
		if(m_bWallJump) {
			WallJump2();
		}

		m_bTumble=false;

		if(m_bCharging)
		{
			if(m_AniState_Upper==ZC_STATE_CHARGE)
				Charged();
			m_bCharging = false;
		}

		m_bSlash = false;

		m_bJumpSlashLanding = false;

		// 공중강베기 착지 혹은 시간이 많이 지났으면..
		if(m_bJumpSlash && (m_bLand || (g_pGame->GetTime()-m_bJumpSlashTime>2.f))) {
			m_bJumpSlash = false;
			m_bJumpSlashLanding = true;
		}

		if(m_bShotReturn) {

			m_nShot=0;
			m_bShotReturn=false;
			SetVelocity(GetVelocity().x,GetVelocity().y,0);
			m_bMoving = false;
		} 

		/*
		bool bCheckGuard = false;

		if( (wtype == MWT_KATANA) || (wtype == MWT_DOUBLE_KATANA) ) 
		{
			bCheckGuard = true;
		}
		*/

		if(m_bShot) {

			if(m_bEnterCharge) {

				EnterCharge();
			} 
			else if(m_nShot==4) {

				m_bShot			= false;
				m_bShotReturn	= true;
				m_bPlayDone		= false;

			} 
			else {

				if(m_nShot==5)
				{
					m_nShot=0;
					m_bShot=false;
					m_bJumpDown=false;
				}
				else
				{
					if(!m_bLButtonQueued && m_nShot<5)
					{
						m_bShot=false;
						m_bShotReturn=true;
					}

					if(m_nShot==3) // sword 는 다른 칼들과 달리..3타까지만이고 3타 리턴이 없다..
						if( m_pVMesh->m_SelectWeaponMotionType == eq_wd_sword ) {
							m_nShot=0;
							m_bShot=false;
							m_bShotReturn = false;
						}

				}
			}
		}

		if(m_bJumpShot)
		{
			m_bJumpShot=false;
//			mlog("3 jumpshot false\n");
		}

		m_bSkill=false;
		m_bBlastStand=false;

		if(m_bBlast)
		{
			m_bBlast=false;

			// 버튼을 눌렸으면 airmove 로.. 그렇지 않으면 fall 로 분기

			if(g_pGame->GetTime()-m_fLastJumpPressedTime<0.5f)
				m_bBlastAirmove=true;
			else
				m_bBlastFall=true;
		}

		if(m_bBlastFall && !m_bDrop && m_bLand)
		{
			m_bDrop=true;
			m_bBlastFall=false;
			m_bBlastDrop=true;
			m_fDropTime=g_pGame->GetTime();
		}

		if(m_bStun && m_nStunType!=ZST_LOOP) {
			m_bStun=false;
		}
	}

#ifdef _DEBUG
	if(m_bGuardTest)
	{
		m_fLastShotTime=g_pGame->GetTime();
	}
#endif

	if(m_bGuard && !m_bGuardCancel 
#ifdef _DEBUG
		&& !m_bGuardTest
#endif
		) {

		bool bGuardCancel = false;

		if(m_bGuardByKey) {//키에 의한 것인지 구분해야 한다..
			if(m_bGuardKey==false)
				bGuardCancel = true;
		}
		else {
			if( (!m_bLButtonPressed || !m_bRButtonPressed) )  {
				bGuardCancel = true;
			}
		}

		if(bGuardCancel)
		{
			m_nGuardBlock = 0;
			m_bGuardStart = false;
			m_bGuardCancel = true;
			m_bGuardKey = false;
			m_bGuardByKey = false;
			SetAnimationUpper(ZC_STATE_UPPER_NONE);
	//		mlog("guard cancel %d %d !\n",m_bLButtonPressed , m_bRButtonPressed);
		}
	}

	if(m_bGuard)
	{
		if(m_nGuardBlock==0 && !m_bGuardBlock_ret && g_pGame->GetTime()-m_fLastShotTime<0.2f)
		{
			m_nGuardBlock=1;
		}
	}

	if(m_bDrop && (g_pGame->GetTime()-m_fDropTime>1.f))
	{
		m_bDrop=false;
		m_bBlastDrop=false;
		m_bBlastStand=true;
	}



	////////////////////////////////////////////////////////////////////
	///////////////////////// 애니메이션 일정 프레임에 의존적인 부분들
	//
	//
	// 천장이 낮은곳에서띄워진 경우 어색해보여서 15프레임이후로는 스킵가능토록한다.
	// TODO : 보다 근본적인 해결이 필요할듯.
	
//	AniFrameInfo* pAniLow = m_pVMesh->GetFrameInfo(ani_mode_lower);
//	AniFrameInfo* pAniUp = m_pVMesh->GetFrameInfo(ani_mode_upper);
	
	if(m_bBlastFall && m_bLand && pAniLow->m_nFrame > 160*15)
	{
		m_bDrop=true;
		m_bBlastFall=false;
		m_bBlastDrop=true;
		m_fDropTime=g_pGame->GetTime();
	}

	// 모으기 53/60 프레임 정도의 위치를 지나면 모으기가 성공한것으로 보자
	if(m_bCharging && !m_bCharged && m_AniState_Lower==ZC_STATE_CHARGE &&
		pAniLow->m_nFrame > 160*52)
		Charged();

	// 점프 강베기시 땅에 닿아있으면 프레임 스킵
	if(m_bJumpSlash && m_bLand && pAniLow->m_nFrame > 160*11)
	{
		m_bJumpSlash = false;
		m_bJumpSlashLanding = true;
	}

//	if(m_bBlastDrop && GetDistToFloor()<5.f && GetVelocity().z<1 && !m_bDrop)

	UpdateAnimation();

	ZCharacter::OnUpdate(fDelta);

	m_bPlayDone = pAniLow->m_isPlayDone;//m_pVMesh->m_isPlayDone[ani_mode_lower];
	m_bPlayDone_upper= 
		pAniUp->m_pAniSet==NULL ? false : pAniUp->m_isPlayDone;
//		m_pVMesh->m_pAniSet[ani_mode_upper]==NULL ? false : m_pVMesh->m_isPlayDone[ani_mode_upper];

	ProcessDelayedWork();

	_EP("ZMyCharacter::Update");
}

void ZMyCharacter::Animation_Reload()
{
	m_bPlayDone_upper = false;
	SetAnimationUpper(ZC_STATE_UPPER_RELOAD);
}
//새로 추가 하려면 디버깅이 너무 많은 시간이 걸릴수도..

void ZMyCharacter::ReserveDashAttacked(MUID uid, float time,rvector &dir)
{
	m_uidReserveDashAttacker = uid;
	m_bReserveDashAttacked = true;
	m_vReserveDashAttackedDir = dir;
	m_fReserveDashAttackedTime = time;
}

void ZMyCharacter::OnDashAttacked(rvector &dir)
{
	if(m_bBlast || m_bBlastDrop || m_bBlastStand || isInvincible())
		return;

	// 속도를 없애는 것들은.. false 해줘야 뜬다.
	m_bSkill=false;
	m_bStun=false;
	m_bShot=false;

	m_bBlast=true;
	m_nBlastType = 1;//모션구분~

	AddVelocity(rvector(dir.x*2000.f,dir.y*2000.f,dir.z*2000.f));
	Normalize(dir);

	float fRatio = GetMoveSpeedRatio();

	AddVelocity(dir * RUN_SPEED * fRatio);
	
	SetLastThrower(m_uidReserveDashAttacker, g_pGame->GetTime()+1.0f);

	m_bWallHang = false;
	m_bHangSuccess = false;

	m_bSlash = false;
	m_bJumpSlash = false;
	m_bJumpSlashLanding = false;

//  Dash 에 의해서 넘어지는 경우에 데미지 주기..
//	int nDamage = ; 당한 무기의 150% 데미지 + 
//	ZPostDamage(m_UID,50);

}

void ZMyCharacter::OnBlast(rvector &dir)
{
	if(m_bBlast || m_bBlastDrop || m_bBlastStand || isInvincible())
		return;

	// 속도를 없애는 것들은.. false 해줘야 뜬다.
	m_bSkill=false;
	m_bStun=false;
	m_bShot=false;

	m_bBlast=true;
	m_nBlastType = 0;

	m_bLand = false;

	SetVelocity( dir * 300.f + rvector(0,0,BLAST_VELOCITY) );

	m_bWallHang = false;
	m_bHangSuccess = false;

	m_bSlash = false;
	m_bJumpSlash = false;
	m_bJumpSlashLanding = false;

	// 막기헤제
	m_bGuard = false;
	SetAnimationUpper(ZC_STATE_UPPER_NONE);
}

void ZMyCharacter::OnTumble(int nDir)
{
#define SWORD_DASH		1000.f
#define GUN_DASH        900.f
	if(IsDie() || m_bWallJump || m_bGuard || m_bDrop || m_bWallJump2 || m_bTumble || m_bWallHang ||
		m_bBlast || m_bBlastFall || m_bBlastDrop || m_bBlastStand || m_bBlastAirmove || 
		m_bCharging || m_bSlash || m_bJumpSlash || m_bJumpSlashLanding ||
		m_bStun || m_AniState_Lower == ZC_STATE_LOWER_UPPERCUT ) return;

	// 공중 텀블링 막았다가 사람들의 아우성으로 주석처리함. -_-;
	//if (!m_bLand) return;

	if(m_bLimitTumble) // 제한이 걸려 있다면~
		return;

//	if( UsingStamina( ZUS_Tumble )==false )	return;

	rvector right;
	rvector forward=RCameraDirection;
	forward.z=0;
	Normalize(forward);
	CrossProduct(&right,rvector(0,0,1),forward);

	float fSpeed;
	if (GetItems()->GetSelectedWeapon()!=NULL &&
		(GetItems()->GetSelectedWeapon()->IsEmpty() == false) &&
		GetItems()->GetSelectedWeapon()->GetDesc()->m_nType==MMIT_MELEE) 
	{
		fSpeed=SWORD_DASH;

		rvector vPos = GetPosition();
		rvector vDir = RealSpace2::RCameraDirection;
		rvector vRight;

		CrossProduct(&vRight,RealSpace2::RCameraUp,RealSpace2::RCameraDirection);

		if(nDir==0) {
			vDir = -RealSpace2::RCameraDirection;
		} else if(nDir==1) {
			vDir = RealSpace2::RCameraDirection;
		} else if(nDir==2) {
			vDir = -vRight;
		} else if(nDir==3) {
			vDir = vRight;
		}

		vDir.z = 0.f;
		Normalize(vDir);

		int sel_type = GetItems()->GetSelectedWeaponParts();

		if( !m_bWallHang && !m_bSkill && !m_bShot && !m_bShotReturn)
			ZPostDash(vPos,vDir,sel_type);

//		ZGetEffectManager()->AddDashEffect(vPos,vDir,this);
	}
	else
	{
		fSpeed=GUN_DASH;
	}

	switch(nDir)
	{
	case 0 :  SetVelocity(forward*fSpeed);break;
	case 1 :  SetVelocity(-forward*fSpeed);break;
	case 2 :  SetVelocity(right*fSpeed);break;
	case 3 :  SetVelocity(-right*fSpeed);break;
	}

	m_bTumble=true;
	m_nTumbleDir=nDir;
	m_bSpMotion = false;
}

void ZMyCharacter::InitStatus()
{
	m_f1ShotTime = 0;
	m_fSkillTime = 0;

	m_fDeadTime=0;

	m_bHero = true;
	m_fLastJumpPressedTime=-1.f;
	m_fJump2Time=0.f;
	m_bWallJump2=false;
	m_bLand=true;
	m_bWallJump=false;
	m_bJumpUp = false;
	m_bJumpDown = false;
	m_bWallHang = false;
	m_bHangSuccess = false;
	m_bTumble = false;
	m_bShot = false;
	m_bShotReturn = false;
	m_nShot=0;
//	m_bLButtonReleased = false;
	m_fLastLButtonPressedTime = -1.f;
	ReleaseLButtonQueue();

	m_bJumpShot = false;
	m_bSkill = false;
	m_bJumpQueued = false;
	m_bDrop = false;

	m_nJumpShotType = 0;

	m_bSplashShot = false;
	m_fSplashShotTime = 0.f;

	m_bGuard = false;
	m_nGuardBlock = 0;
	m_bGuardStart = false;
	m_bGuardCancel = false;

	m_fLastShotTime=0.f;
	m_fNextShotTime=0.f;

	m_bSlash = false;
	m_bJumpSlash = false;
	m_bJumpSlashLanding = false;

	m_bEnterCharge = false;

	ZCharacter::InitStatus();

}

void ZMyCharacter::InitBullet()
{
	ZCharacter::InitBullet();

	MDataChecker* pChecker = ZApplication::GetGame()->GetDataChecker();

	// 총알 초기화
	if (!m_Items.GetItem(MMCIP_PRIMARY)->IsEmpty()) 
	{
		pChecker->RenewCheck((BYTE*)m_Items.GetItem(MMCIP_PRIMARY)->GetBulletPointer(), sizeof(int));
		pChecker->RenewCheck((BYTE*)m_Items.GetItem(MMCIP_PRIMARY)->GetAMagazinePointer(), sizeof(int));
	}
	if (!m_Items.GetItem(MMCIP_SECONDARY)->IsEmpty()) 
	{
		pChecker->RenewCheck((BYTE*)m_Items.GetItem(MMCIP_SECONDARY)->GetBulletPointer(), sizeof(int));
		pChecker->RenewCheck((BYTE*)m_Items.GetItem(MMCIP_SECONDARY)->GetAMagazinePointer(), sizeof(int));
	}
	if (!m_Items.GetItem(MMCIP_CUSTOM1)->IsEmpty()) 
	{
		pChecker->RenewCheck((BYTE*)m_Items.GetItem(MMCIP_CUSTOM1)->GetBulletPointer(), sizeof(int));
		pChecker->RenewCheck((BYTE*)m_Items.GetItem(MMCIP_CUSTOM1)->GetAMagazinePointer(), sizeof(int));
	}
	if (!m_Items.GetItem(MMCIP_CUSTOM2)->IsEmpty()) 
	{
		pChecker->RenewCheck((BYTE*)m_Items.GetItem(MMCIP_CUSTOM2)->GetBulletPointer(), sizeof(int));
		pChecker->RenewCheck((BYTE*)m_Items.GetItem(MMCIP_CUSTOM2)->GetAMagazinePointer(), sizeof(int));
	}
}

// 나 자신일 경우에는 카메라도 같이 설정한다.
void ZMyCharacter::SetDirection(const rvector& dir)
{
	ZCharacter::SetDirection(dir);
	ZCamera* pCamera = ZApplication::GetGameInterface()->GetCamera();
	pCamera->SetDirection(dir);
}

void ZMyCharacter::OnDamagedAnimation(ZObject *pAttacker,int type)
{
	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_SKILLMAP)
		return;

	ZCharacter::OnDamagedAnimation(pAttacker,type);

	m_bShot=false;
	m_bWallHang=false;
	m_bCharging=false;
	m_bSkill=false;
}

void ZMyCharacter::OnDie()
{
	ZCharacter::OnDie();
	m_fDeadTime=g_pGame->GetTime();

	m_bWallHang=false;
	m_bWallJump=false;
}

#define DAMAGE_VELOCITY		1700.f			// 속도가 이 이상되면 폴링데미지를 받는다.
#define MAX_FALL_SPEED		3000.f			// 최대 낙하속도
#define MAX_FALL_DAMAGE		50.f			// 최대 데미지
#define BLASTED_KNOCKBACK_RATIO	3.f			// 띄워진상태에서의 Knockback을 과장(배수)


//void ZMyCharacter::UpdatePosition(float fDelta)
//{
//	// 실제 움직이는 부분은 module_movable 로 옮겨갔다
//
//}


#define CA_FACTOR_PISTOL		0.3f
#define CA_FACTOR_SMG			0.3f
#define CA_FACTOR_REVOLVER		0.35f
#define CA_FACTOR_SHOTGUN		1.0f
#define CA_FACTOR_RIFLE			0.25f
#define CA_FACTOR_MACHINEGUN	1.0f
#define CA_FACTOR_ROCKET		1.0f

float ZMyCharacter::GetControllabilityFactor()
{
	MMatchWeaponType wtype = MWT_NONE;

	ZItem* pSItem = GetItems()->GetSelectedWeapon();

	if(pSItem) {
		if( pSItem->GetDesc() )
			wtype = pSItem->GetDesc()->m_nWeaponType;
	}

	{
		switch (wtype)
		{

		case MWT_PISTOL: 
		case MWT_PISTOLx2:
			{
				return CA_FACTOR_PISTOL;
			}
			break;

		case MWT_REVOLVER:
		case MWT_REVOLVERx2:
			{
				return CA_FACTOR_REVOLVER;
			}
			break;

		case MWT_SMG:
		case MWT_SMGx2:
			{
				return CA_FACTOR_SMG;
			}
			break;

		case MWT_SHOTGUN:
		case MWT_SAWED_SHOTGUN:
			{
				return CA_FACTOR_SHOTGUN;
			}
			break;

		case MWT_MACHINEGUN:
			{
				return CA_FACTOR_MACHINEGUN;
			}
			break;

		case MWT_RIFLE:
		case MWT_SNIFER:
			{
				return CA_FACTOR_RIFLE;
			}
			break;

		case MWT_ROCKET:
			{
				return CA_FACTOR_ROCKET;
			}
			break;
		}
	}

	return 1.0f;
}

void ZMyCharacter::UpdateCAFactor(float fDelta)
{
	const float fCurrWeaponCAFactor = GetControllabilityFactor();
	bool bPressed= ZIsActionKeyPressed(ZACTION_USE_WEAPON);
	if (bPressed) return;

	m_fElapsedCAFactorTime += fDelta;
	while (m_fElapsedCAFactorTime > CAFACTOR_DEC_DELAY_TIME)
	{
		m_fCAFactor -= fCurrWeaponCAFactor;
		m_fElapsedCAFactorTime -= CAFACTOR_DEC_DELAY_TIME;
	}

	if (m_fCAFactor < fCurrWeaponCAFactor) m_fCAFactor = fCurrWeaponCAFactor;
}




#ifndef _PUBLISH

#include "Physics.h"

////////////////////////////////////////////////////////////
ZDummyCharacter::ZDummyCharacter() : ZMyCharacter()
{
	// 랜덤으로 아무거나 입도록 만든다
	#define _DUMMY_CHARACTER_PRESET		5
	unsigned long int nMeleePreset[_DUMMY_CHARACTER_PRESET] = {1, 11, 3, 14, 15};
	unsigned long int nPrimaryPreset[_DUMMY_CHARACTER_PRESET] = {4010, 4013, 5004, 6004, 9001};
	unsigned long int nSecondaryPreset[_DUMMY_CHARACTER_PRESET] = {9003, 9004, 9006, 7002, 6006};
	unsigned long int nChestPreset[_DUMMY_CHARACTER_PRESET] = {21001, 21002, 21004, 21005, 21006};
	unsigned long int nLegsPreset[_DUMMY_CHARACTER_PRESET] = {23001, 23005, 23002, 23004, 23003};
	unsigned long int nHandsPreset[_DUMMY_CHARACTER_PRESET] = {22001, 22002, 22003, 22004, 22501};
	unsigned long int nFeetPreset[_DUMMY_CHARACTER_PRESET] = {24001, 24002, 24003, 24004, 24005};


	static int m_stIndex = 0; m_stIndex++;

	MTD_CharInfo info;
	char szTempName[128]; sprintf_safe(szTempName, "noname%d", m_stIndex); strcpy_safe(info.szName, szTempName);
	info.nSex = 0;

	for (int j=0; j < MMCIP_END; j++) info.nEquipedItemDesc[j] = 0;
	info.nEquipedItemDesc[MMCIP_MELEE] = nMeleePreset[RandomNumber(0, 4)];
	info.nEquipedItemDesc[MMCIP_PRIMARY] = nPrimaryPreset[RandomNumber(0, 4)];
	info.nEquipedItemDesc[MMCIP_SECONDARY] = nSecondaryPreset[RandomNumber(0, 4)];
	info.nEquipedItemDesc[MMCIP_CHEST] = nChestPreset[RandomNumber(0, 4)];
	info.nEquipedItemDesc[MMCIP_HANDS] = nHandsPreset[RandomNumber(0, 4)];
	info.nEquipedItemDesc[MMCIP_LEGS] = nLegsPreset[RandomNumber(0, 4)];
	info.nEquipedItemDesc[MMCIP_FEET] = nFeetPreset[RandomNumber(0, 4)];

	Create(info);
	SetVisible(true);

	m_Items.GetItem(MMCIP_PRIMARY)->InitBullet(999999);
	m_Items.GetItem(MMCIP_SECONDARY)->InitBullet(999999);
	m_Items.SelectWeapon(MMCIP_PRIMARY);
	m_Items.GetItem(MMCIP_PRIMARY)->Reload();


	m_fNextAniTime = 5.0f;
	m_fElapsedTime = 0.0f;
	m_fNextShotTime = 5.0f;
	m_fShotElapsedTime = 0.0f;
	m_fShotDelayElapsedTime = 0.0f;
	m_bShotting = false;
	m_bShotEnable = false;
}

ZDummyCharacter::~ZDummyCharacter()
{

}
#include "Physics.h"

void ZDummyCharacter::OnUpdate(float fDelta)
{
	ZCharacter::OnUpdate(fDelta);



	m_fElapsedTime += fDelta;
	m_fShotDelayElapsedTime += fDelta;
	m_fShotElapsedTime += fDelta;


	if (m_fElapsedTime >= m_fNextAniTime)
	{
		int nAni = rand() % ZC_STATE_LOWER_END;
		SetAnimationLower(ZC_STATE_LOWER(nAni));

		m_fNextAniTime = RandomNumber(2.0f, 6.0f);
		m_fElapsedTime = 0.0f;

	}


	if (m_fShotElapsedTime >=  m_fNextShotTime)
	{
		m_bShotting = !m_bShotting;
		m_fNextShotTime = RandomNumber(3.0f, 10.0f);
		m_fShotElapsedTime = 0.0f;
	}

	if ((m_bShotEnable) && (m_bShotting))
	{
		if (m_fShotDelayElapsedTime >= ((float)m_Items.GetItem(MMCIP_PRIMARY)->GetDesc()->m_nDelay / 1000.0f))
		{
			m_Items.GetItem(MMCIP_PRIMARY)->Reload();
			float fShotTime=g_pGame->GetTime();
			g_pGame->OnPeerShot(m_UID, fShotTime, m_Position, m_Direction, GetItems()->GetSelectedWeaponParts());
			m_fShotDelayElapsedTime = 0.0f;
		}

	}
}


#endif // _PUBLISH

bool ZMyCharacter::IsGuard() const
{
	return m_bGuard && !m_bGuardStart;
}

bool ZMyCharacter::IsGuardCustom() const
{
#ifdef GUARD_START_CAN_BLOCK
	return m_bGuard;
#else
	return IsGuard();
#endif
}

void ZMyCharacter::ProcessDelayedWork()
{
	for(ZDELAYEDWORKLIST::iterator i = m_DelayedWorkList.begin(); i != m_DelayedWorkList.end();i++)
	{
		ZDELAYEDWORKITEM *pItem = *i;

		// 실행할 시간이 지났으면 실행한다
		if(g_pGame->GetTime() > pItem->fTime) 
		{
			OnDelayedWork(pItem);
			i = m_DelayedWorkList.erase(i);
			delete pItem;

			if (i == m_DelayedWorkList.end())
				break;
		}
	}
}

void ZMyCharacter::AddDelayedWork(float fTime,ZDELAYEDWORK nWork, void *Data)
{
	ZDELAYEDWORKITEM *pItem = new ZDELAYEDWORKITEM;
	pItem->fTime = fTime;
	pItem->nWork = nWork;
	pItem->Data = Data;

	m_DelayedWorkList.push_back(pItem);
}

void ZMyCharacter::OnDelayedWork(ZDELAYEDWORKITEM *pItem)
{
	switch(pItem->nWork) {
	case ZDW_RECOIL :
//		if(m_bShot) 
		{
			// 막고있는 사람이 있나 체크해봐서 있으면 스턴을 먹는다
			for (ZCharacterManager::iterator itor = ZGetCharacterManager()->begin();
				itor != ZGetCharacterManager()->end(); ++itor)
			{
				ZCharacter* pTar = (*itor).second;
				if (this == pTar || pTar->IsDie()) continue;

				rvector diff = GetPosition() + m_Direction*10.f - pTar->GetPosition();
				diff.z *= .5f;
				float fDist = Magnitude(diff);

				if (fDist < 200.0f) {

					bool bCheck = false;

					if (ZApplication::GetGame()->GetMatch()->IsTeamPlay()){
						if( IsTeam( pTar ) == false){
							bCheck = true;
						}
					}
					else {
						bCheck = true;
					}

					if(g_pGame->CheckWall(this,pTar)==true) //중간에 벽이 막고 있는가?
						bCheck = false;

					if( bCheck) {//팀이아닌경우만

						rvector fTarDir = pTar->GetPosition() - GetPosition();
						Normalize(fTarDir);
						float fDot = D3DXVec3Dot(&m_Direction, &fTarDir);
						if (fDot>0.1 && DotProduct(m_Direction,pTar->m_Direction)<0)	// 방향도 맞아야지만
						{
							if(pTar->IsGuard()) 
								ShotBlocked();
						}
					}
				}
			}
		}
		break;
	case ZDW_UPPERCUT :
		if(m_bSkill) {	// 아직 상태를 벗어나지 않았다면,,
			MMatchWeaponType type = MWT_NONE;

			int sel_type = GetItems()->GetSelectedWeaponParts();
			ZItem* pSItem = GetItems()->GetSelectedWeapon();

			if(pSItem && pSItem->GetDesc())
				type = pSItem->GetDesc()->m_nWeaponType;

			if(type == MWT_KATANA || type == MWT_DOUBLE_KATANA ) {
				ZPostSkill( g_pGame->GetTime(), ZC_SKILL_UPPERCUT , sel_type );
			}
		}
		break;
	case ZDW_DASH :
		if(m_bSkill) {	// 아직 상태를 벗어나지 않았다면,,
			MMatchWeaponType type = MWT_NONE;

			int sel_type = GetItems()->GetSelectedWeaponParts();
			ZItem* pSItem = GetItems()->GetSelectedWeapon();

			if(pSItem && pSItem->GetDesc())
				type = pSItem->GetDesc()->m_nWeaponType;

			if(type == MWT_DAGGER) {//한손대거..
				ZPostSkill( g_pGame->GetTime(), ZC_SKILL_DASH , sel_type );
			}
			else if(type == MWT_DUAL_DAGGER) {//양손대거..
				ZPostSkill( g_pGame->GetTime(), ZC_SKILL_DASH , sel_type );
			}
		}
		break;
	case ZDW_MASSIVE:
		Discharged();
		if(m_bSlash || m_bJumpSlash)
			ZPostSkill(g_pGame->GetTime(), ZC_SKILL_SPLASHSHOT, GetItems()->GetSelectedWeaponParts());
		break;
	case ZDW_RG_SLASH:
		ZPostRGSlash(m_Position, m_Direction, (int)pItem->Data);
		break;
	case ZDW_RG_MASSIVE:
		Discharged();
		if (m_bSlash || m_bJumpSlash)
			ZPostRGMassive(m_Position, m_Direction);
		break;
	case ZDW_RG_RECOIL:
	{
		rvector AttackerPosition = GetPosition();
		rvector AttackerDirection = GetDirection();

		rvector HAttackerDirection = AttackerDirection;
		HAttackerDirection.z = 0;
		Normalize(HAttackerDirection);

		ZCharacter *pTarget = (ZCharacter *)pItem->Data;

		rvector TargetPosition = pTarget->GetPosition();

		rvector HVectorToTarget = AttackerPosition - TargetPosition;
		HVectorToTarget.z = 0;
		float dist = Magnitude(HVectorToTarget);
		if (dist > (g_Rules.CanTipslash() ? 240 : 400))
			return;

		rvector HDirToTarget = TargetPosition - AttackerPosition;
		HDirToTarget.z = 0;
		Normalize(HDirToTarget);

		if (DotProduct(HAttackerDirection, HDirToTarget) < 0.5) // angle > 60 degrees
			return;

		rvector TargetDirection = pTarget->GetDirection();

		if (DotProduct(AttackerDirection, TargetDirection) > 0) // angle < 90 degrees
			return;

		ShotBlocked();
	}
		break;
	default:
		MLog("Invalid delayed work %d\n", pItem->nWork);
	}
}

void ZMyCharacter::AddRecoilTarget(ZCharacter *pTarget)
{
	AddDelayedWork(ZGetGame()->GetTime() + 0.1, ZDW_RG_RECOIL, pTarget);
}

void ZMyCharacter::EnterCharge()
{
	if(!m_bCharging)
		ZPostReaction(g_pGame->GetTime(), ZR_CHARGING);

	m_bShot	= false;
	m_nShot	= 0;
	m_bCharging = true;
	m_bPlayDone		= false;
	m_bEnterCharge = false;
}

void ZMyCharacter::ChargedShot()
{
	m_bSlash = true;
	m_bTumble = false;
	SetVelocity(0,0,0);

	ReleaseLButtonQueue();

	if(g_Rules.IsVanillaMode())
	{
		AddDelayedWork(g_pGame->GetTime() + 0.25f, ZDW_RECOIL);
		AddDelayedWork(g_pGame->GetTime() + 0.3f, ZDW_MASSIVE);
	}
	else
	{
		AddDelayedWork(g_pGame->GetTime() + 0.3 + 0.15, ZDW_RG_MASSIVE);
	}
}

void ZMyCharacter::JumpChargedShot()
{
	m_bWallJump = false;
	m_bWallJump2 = false;
	m_bJumpSlash = true;
	m_bTumble = false;
	m_bCharged = false;
	ReleaseLButtonQueue();

	if (g_Rules.IsVanillaMode())
	{
		AddDelayedWork(g_pGame->GetTime() + 0.15f, ZDW_RECOIL);
		AddDelayedWork(g_pGame->GetTime() + 0.2f, ZDW_MASSIVE);
	}
	else
	{
		AddDelayedWork(g_pGame->GetTime() + 0.2 + 0.15, ZDW_RG_MASSIVE);
	}

	m_bJumpSlashTime = g_pGame->GetTime();
}

void ZMyCharacter::ShotBlocked()
{
	m_bStun = true;
	SetVelocity(0,0,0);
	m_nStunType = ZST_BLOCKED;
	m_bShot = false;
	m_bSlash = false;
	m_bJumpSlash = false;
}

void ZMyCharacter::Charged()				// 기 모았다
{
	m_bCharged = true;
	m_fChargedFreeTime = g_pGame->GetTime() + CHARGED_TIME;

	ZPostReaction(CHARGED_TIME,ZR_CHARGED);
}

void ZMyCharacter::OnMeleeGuardSuccess()
{
	
	// 가드를 풀어준다
	m_fGuardStartTime = g_pGame->GetTime() - GUARD_DURATION; // 다시 가드가 안되도록 
	m_bGuardCancel = true;
	m_bCharged = true;
	m_fChargedFreeTime = g_pGame->GetTime() + COUNTER_CHARGED_TIME;

	ZPostReaction(COUNTER_CHARGED_TIME,ZR_CHARGED);

	// 사운드
	ZCharacter::OnMeleeGuardSuccess();
}

void ZMyCharacter::Discharged()			// 기모으기 풀렸을때
{
	m_bCharged = false;
	ZPostReaction(g_pGame->GetTime(),ZR_DISCHARGED);
}

float ZMyCharacter::GetGravityConst()	// 중력의 영향을 얼만큼 받는지..
{
	// 매달려있다
	if(m_bWallHang && m_bHangSuccess) return 0;
	if(m_bShot) return 0;

	// 띄워진상태면 천천히 ...
	if(m_bBlastFall) return .7f;

	// 벽점프 중이면 천천히 내려온다.
	if(m_bWallJump)
	{
		if(m_nWallJumpDir==1 || GetVelocity().z<0)
			return 0;
		else
			return .1f;
	}

	// 강베기 앞의 점프 모션동안 영향을 받지 않는다
	if(m_bSlash)
	{
		MMatchItemDesc *pDesc = GetItems()->GetItem(MMCIP_MELEE)->GetDesc();
		if(pDesc->m_nWeaponType==MWT_DOUBLE_KATANA) {
			AniFrameInfo* pAniLow = m_pVMesh->GetFrameInfo(ani_mode_lower);
			if( pAniLow->m_nFrame < 160*11) return 0;
		}
	}

	// 쌍검 특수동작 점프 모션
	if(m_bSkill) {
		MMatchItemDesc *pDesc = GetItems()->GetItem(MMCIP_MELEE)->GetDesc();
		if(pDesc->m_nWeaponType==MWT_DOUBLE_KATANA) {
			AniFrameInfo* pAniLow = m_pVMesh->GetFrameInfo(ani_mode_lower);
			if( pAniLow->m_nFrame < 160*20) return 0;
		}
	}

	return 1.f;
}

void ZMyCharacter::OnGuardSuccess()
{
	m_fLastShotTime=g_pGame->GetTime();
}

void ZMyCharacter::OnDamaged(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio, int nMeleeType)
{
	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_SKILLMAP)
		return;

	ZCharacter::OnDamaged(pAttacker,srcPos,damageType,weaponType,fDamage,fPiercingRatio,nMeleeType);
	ZGetScreenEffectManager()->AddAlert(GetPosition(),m_Direction, srcPos);

	if(damageType==ZD_EXPLOSION)
	{
		if (GetVelocity().z > 0 && pAttacker!=NULL )
			SetLastThrower(pAttacker->GetUID(), g_pGame->GetTime()+1.0f);
	}

	LastDamagedTime = ZGetGame()->GetTime();
}

void ZMyCharacter::OnKnockback(const rvector& dir, float fForce)
{
	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_SKILLMAP)
		return;

	if(m_bBlast || m_bBlastFall) {	
		// 떠있을때 넉백
		rvector vKnockBackDir = dir;
		Normalize(vKnockBackDir);
		vKnockBackDir *= (fForce * BLASTED_KNOCKBACK_RATIO);
		vKnockBackDir.x = vKnockBackDir.x * 0.2f;
		vKnockBackDir.y = vKnockBackDir.y * 0.2f;
		SetVelocity(vKnockBackDir);
	} else {
		// 그냥 넉백
		ZCharacter::OnKnockback(dir, fForce);
	}
}

void ZMyCharacter::ReleaseButtonState()
{
	m_bLButtonFirstPressed = false;
	m_bLButtonPressed = false;

	m_bRButtonFirstReleased = true;
	m_bRButtonFirstPressed = false;
	m_bRButtonPressed = false;

	m_bLButtonQueued = false;
}

void ZMyCharacter::OnStun(float fTime)
{
	m_fStunEndTime = g_pGame->GetTime() + fTime;
	m_bStun = true;
	m_nStunType = ZST_LOOP;
}
