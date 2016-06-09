#include "stdafx.h"
#include "LagCompensation.h"
#include <d3d9.h>
#include "GlobalTypes.h"
#include "RealSpace2.h"
#include "MZFileSystem.h"
#undef pi
#include "MXml.h"


struct ZANIMATIONINFO {
	char *Name;
	bool bEnableCancel;		// 캔슬 가능한지
	bool bLoop;				// 반복 되는 동작
	bool bMove;				// 움직임이 포함된 애니메이션
	bool bContinuos;		// 포함된 움직임이 시작부터 연결되어있는지.
};

static ZANIMATIONINFO g_AnimationInfoTableLower[ZC_STATE_LOWER_END] = {
	{ ""				,true	,true	,false 	,false },

	{ "idle"			,true	,true	,false 	,false },
	{ "idle2"			,true	,true	,false 	,false },
	{ "idle3"			,true	,true	,false 	,false },
	{ "idle4"			,true	,true	,false 	,false },

	{ "run"				,true	,true	,false 	,false },
	{ "runB"			,true	,true	,false 	,false },
	{ "runL"			,true	,true	,false 	,false },
	{ "runR"			,true	,true	,false 	,false },

	{ "jumpU"			,true	,false	,false 	,false },
	{ "jumpD"			,true	,false	,false 	,false },

	{ "die" 			,true	,false	,false 	,false },
	{ "die2" 			,true	,false	,false 	,false },
	{ "die3" 			,true	,false	,false 	,false },
	{ "die4"			,true	,false	,false 	,false },

	{ "runLW"			,false	,true 	,false	,false },
	{ "runLW_down"		,false	,false	,false	,false },
	{ "runW" 			,false	,false	,true	,false },
	{ "runW_downF"		,false	,false	,false	,false },
	{ "runW_downB"		,false	,false	,false	,false },
	{ "runRW" 			,false	,true 	,false	,false },
	{ "runRW_down"		,false	,false 	,false	,false },

	{ "tumbleF"			,false	,false	,false	,false },
	{ "tumbleB"			,false	,false	,false	,false },
	{ "tumbleR"			,false	,false	,false	,false },
	{ "tumbleL"			,false	,false	,false	,false },

	{ "bind"			,false	,false	,false	,false },

	{ "jumpwallF"		,false	,false 	,false	,false },
	{ "jumpwallB"		,false	,false 	,false	,false },
	{ "jumpwallL"		,false	,false 	,false	,false },
	{ "jumpwallR"		,false	,false 	,false	,false },

	{ "attack1"			,false	,false 	,true  	,false },
	{ "attack1_ret"		,false	,false 	,true  	,true },
	{ "attack2"			,false	,false 	,true  	,false },
	{ "attack2_ret"		,false	,false 	,true  	,true },
	{ "attack3"			,false	,false 	,true  	,false },
	{ "attack3_ret"		,false	,false 	,true  	,true },
	{ "attack4"			,false	,false 	,true  	,false },
	{ "attack4_ret"		,false	,false 	,true  	,true },
	{ "attack5"			,false	,false 	,true  	,false },

	{ "attack_Jump"		,false	,false 	,false	,false },
	{ "uppercut"		,false	,false 	,true	,false },

	{ "guard_start"		,false	,false 	,true 	,false },
	{ "guard_idle"		,false	,false 	,false	,false },
	{ "guard_block1"	,false	,false 	,false	,false },
	{ "guard_block1_ret",false	,false 	,false	,false },
	{ "guard_block2"	,false	,false 	,false	,false },
	{ "guard_cancel"	,false	,false 	,false	,false },

	{ "blast"			,false	,false 	,false 	,false },
	{ "blast_fall"		,false	,false 	,false 	,false },
	{ "blast_drop"		,false	,false 	,false 	,false },
	{ "blast_stand"		,false	,false 	,false 	,false },
	{ "blast_airmove"	,false	,false 	,false 	,false },

	{ "blast_dagger"	 ,false ,false 	,false 	,false },
	{ "blast_drop_dagger",false ,false 	,false 	,false },

	{ "damage"			,false	,false 	,false 	,false },
	{ "damage2"			,false	,false 	,false 	,false },
	{ "damage_down"		,false	,false 	,false 	,false },

	{ "taunt"			,true	,false	,false	,false },
	{ "bow"				,true	,false	,false	,false },
	{ "wave"			,true	,false	,false	,false },
	{ "laugh"			,true	,false	,false	,false },
	{ "cry"				,true	,false	,false	,false },
	{ "dance"			,true	,false	,false	,false },

	{ "cancel"			,false	,false 	,false 	,false },
	{ "charge"			,false	,false 	,true  	,true },
	{ "slash"			,false	,false 	,true  	,false },
	{ "jump_slash1"		,false	,false 	,false	,false },
	{ "jump_slash2"		,false	,false 	,false	,false },

	{ "lightning"		,false	,false 	,false	,false },
	{ "stun"			,false	,false 	,false	,false },	// 루프되는 스턴

	{ "pit"				,false	,false 	,false	,false },	// 나락에서 떨어지는 거
};

static ZANIMATIONINFO g_AnimationInfoTableUpper[ZC_STATE_UPPER_END] = {
	{ ""				,true	,true	,false	,false },

	{ "attackS"			,false	,false	,false	,false },
	{ "reload"			,false	,false	,false	,false },
	{ "load"			,false	,false	,false	,false },

	{ "guard_start"		,false	,false 	,false	,false },
	{ "guard_idle"		,false	,false 	,false	,false },
	{ "guard_block1"	,false	,false 	,false	,false },
	{ "guard_block1_ret",false	,false 	,false	,false },
	{ "guard_block2"	,false	,false 	,false	,false },
	{ "guard_cancel"	,false	,false 	,false	,false },
};

bool LagCompManager::Create()
{
	const char* path = "C:/Apache24/htdocs/patch";
	g_pFileSystem = new MZFileSystem();
	
	if (!g_pFileSystem->Create(path))
	{
		MGetMatchServer()->LogF(MMatchServer::LOG_ALL, "g_pFileSystem->Create failed!");
		return false;
	}

	bool ret = false;
	
	ret = LoadAnimations("model/man/man01.xml", 0);
	if (!ret)
	{
		MGetMatchServer()->LogF(MMatchServer::LOG_ALL, "LoadAnimations0 failed!");
		return false;
	}

	LoadAnimations("model/woman/woman01.xml", 1);
	if (!ret)
	{
		MGetMatchServer()->LogF(MMatchServer::LOG_ALL, "LoadAnimations1 failed!");
		return false;
	}

	//for (int AniIdx = 0; AniIdx < ZC_STATE_LOWER_END; AniIdx++)
	//{
	//	auto& AniItem = g_AnimationInfoTableLower[AniIdx];
	//	MGetMatchServer()->LogF(MMatchServer::LOG_ALL, "%s", AniItem.Name);
	//	auto Ani = AniMgrs[0].GetAnimation(AniItem.Name);

	//	if (!Ani)
	//	{
	//		MGetMatchServer()->LogF(MMatchServer::LOG_ALL, "Can't find animation!");
	//		continue;
	//	}

	//	if (!Ani->m_pAniData)
	//	{
	//		MGetMatchServer()->LogF(MMatchServer::LOG_ALL, "Can't find ani data!");
	//		continue;
	//	}

	//	MGetMatchServer()->LogF(MMatchServer::LOG_ALL, "Max frame %d", Ani->m_pAniData->m_max_frame);

	//	/*for (int i = 0; i < Ani->m_pAniData->m_max_frame; i++)
	//	{
	//		auto v = GetHeadPosition(rvector(0, 0, 0), MMatchSex(0), ZC_STATE_LOWER(AniIdx), i);
	//		MGetMatchServer()->LogF(MMatchServer::LOG_ALL, "Frame %d: %f, %f, %f", i, v.x, v.y, v.z);
	//	}*/
	//}

	return true;
}

bool LagCompManager::LoadAnimations(const char* filename, int Index)
{
	auto& AniMgr = AniMgrs[Index];

	MXmlDocument	XmlDoc;
	MXmlElement		DocNode, Node;

	XmlDoc.Create();

	char Path[256];
	Path[0] = NULL;

	GetPath(filename, Path);

	//<--------

	//	if( !XmlDoc.LoadFromFile(filename) ) 
	//		return false;

	char *buffer;
	MZFile mzf;

	if (g_pFileSystem) {
		if (!mzf.Open(filename, g_pFileSystem)) {
			if (!mzf.Open(filename))
			{
				MGetMatchServer()->LogF(MMatchServer::LOG_ALL, "mzf.Open failed with g_pFileSystem!");
				return false;
			}
		}
	}
	else {
		if (!mzf.Open(filename))
		{
			MGetMatchServer()->LogF(MMatchServer::LOG_ALL, "mzf.Open failed without g_pFileSystem!");
			return false;
		}
	}

	buffer = new char[mzf.GetLength() + 1];
	buffer[mzf.GetLength()] = 0;

	mzf.Read(buffer, mzf.GetLength());

	if (!XmlDoc.LoadFromMemory(buffer))
	{
		MGetMatchServer()->LogF(MMatchServer::LOG_ALL, "XmlDoc.LoadFromMemory failed!");
		return false;
	}

	delete[] buffer;

	mzf.Close();

	//-------->

	DocNode = XmlDoc.GetDocumentElement();

	char NodeName[256];
	char IDName[256];
	char FileName[256];
	char PathFileName[256];
	char MotionTypeID[256];
	char MotionLoopTypeID[256];
	char SoundFileName[256];
	char GameMotion[256];
	char PathSoundFileName[256];

	int nMTypeID = -1;
	bool bSoundMap = false;
	AnimationLoopType MLoopType = RAniLoopType_Loop;

	int nCnt = DocNode.GetChildNodeCount();

	for (int i = 0; i<nCnt; i++) {

		Node = DocNode.GetChildNode(i);

		Node.GetTagName(NodeName);

		if (NodeName[0] == '#') continue;

		if (strcmp(NodeName, "AddAnimation") == 0) {

			SoundFileName[0] = NULL;
			Node.GetAttribute(IDName, "name");
			Node.GetAttribute(FileName, "filename");
			Node.GetAttribute(MotionTypeID, "motion_type");
			Node.GetAttribute(MotionLoopTypeID, "motion_loop_type");
			Node.GetAttribute(SoundFileName, "sound");
			Node.GetAttribute(&bSoundMap, "soundmap", false);
			Node.GetAttribute(GameMotion, "gm");

			if (strcmp(MotionLoopTypeID, "normal") == 0) {
				MLoopType = RAniLoopType_Normal;
			}
			else if (strcmp(MotionLoopTypeID, "loop") == 0) {
				MLoopType = RAniLoopType_Loop;
			}
			else if (strcmp(MotionLoopTypeID, "onceidle") == 0) {
				MLoopType = RAniLoopType_OnceIdle;
			}
			else if (strcmp(MotionLoopTypeID, "lastframe") == 0) {
				MLoopType = RAniLoopType_HoldLastFrame;
			}
			else if (strcmp(MotionLoopTypeID, "onceLowerbody") == 0) {
				MLoopType = RAniLoopType_OnceLowerBody;
			}

			//MLog("Name: %s, filename: %s, motion_type %s, motion_loop_type %s\n", IDName, FileName, MotionTypeID, MotionLoopTypeID);

			int nGameMotion = atoi(GameMotion);

			//if (GetToolMesh()) // 툴에서 사용되는거라면 무조건 바로 로딩~
			//	nGameMotion = 0;

			nMTypeID = atoi(MotionTypeID);

			if (Path[0]) {
				strcpy_safe(PathFileName, Path);
				strcat(PathFileName, FileName);
			}
			else
				strcpy_safe(PathFileName, FileName);

			RAnimation* pAni = NULL;

			//MLog("AddAnimation %s, %s\n", IDName, PathFileName);

			//if (nGameMotion == 1) { // 게임 모션은 나중에 로딩
			//	pAni = AniMgr.AddGameLoad(IDName, PathFileName, -1, nMTypeID);
			//}
			//else
			{
				pAni = AniMgr.Add(IDName, PathFileName, -1, nMTypeID);
			}

			// NOTE: Hack to slow down massives
			if (!strcmp(IDName, "slash"))// || !strncmp(IDName, "jump_slash", 10))
			{
				pAni->PlaybackRate = 0.9;
			}

			if (pAni) {

				pAni->SetAnimationLoopType(MLoopType);

				if (SoundFileName[0] == NULL) {//사운드가 등록되지 않았으면 에니메이션 파일 이름이 기본 사운드이름..
					int len = (int)strlen(FileName);
					strncpy(SoundFileName, FileName, len - 8);//.elu.ani 생략...
					SoundFileName[len - 8] = NULL;

					strcpy_safe(PathSoundFileName, "/sound/effect/");
					strcat(PathSoundFileName, SoundFileName);
				}

				pAni->SetSoundFileName(SoundFileName);
				pAni->SetSoundRelatedToMap(bSoundMap);
			}
		}
	}

	XmlDoc.Destroy();

	return true;
}

matrix LagCompManager::GetHeadMatrix(MMatchSex Sex, ZC_STATE_LOWER v, int Frame)
{
	auto Ani = AniMgrs[Sex].GetAnimation(g_AnimationInfoTableLower[v].Name);

	if (!Ani)
	{
		MGetMatchServer()->LogF(MMatchServer::LOG_ALL, "GetHeadPosition -- Can't find animation!");
		return v3(0, 0, 0);
	}

	auto Node = Ani->m_pAniData->GetNode("Bip01 Head");

	if (!Node)
	{
		MGetMatchServer()->LogF(MMatchServer::LOG_ALL, "GetHeadPosition -- Can't find head node!");
		return v3(0, 0, 0);
	}

	auto Rot = Node->GetRotValue(Frame);

	matrix mat;
	D3DXMatrixRotationQuaternion(&mat, &Rot);

	/*auto Pos = Node->GetPosValue(Frame);

	for (int i = 0; i < 3; i++)
		mat(3, i) = Pos[i];*/

	auto Neck = Ani->m_pAniData->GetNode("Bip01 Head");

	matrix NeckInv;
	RMatInv(NeckInv, Neck->m_mat_base);

	auto Pos = GetTransPos(Node->m_mat_base * NeckInv);

	for (int i = 0; i < 3; i++)
		mat(3, i) = Pos[i];

	return mat;
}