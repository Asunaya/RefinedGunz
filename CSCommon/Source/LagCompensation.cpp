#include "stdafx.h"
#include "LagCompensation.h"
#include <d3d9.h>
#include "GlobalTypes.h"
#include "RealSpace2.h"
#include "MZFileSystem.h"
#undef pi
#include "MXml.h"
#include "MUtil.h"
#include "MMatchConfig.h"

bool LagCompManager::Create()
{
	const char* path = MGetServerConfig()->GetGameDirectory();
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

	ret = LoadAnimations("model/woman/woman01.xml", 1);
	if (!ret)
	{
		MGetMatchServer()->LogF(MMatchServer::LOG_ALL, "LoadAnimations1 failed!");
		return false;
	}

	SetAnimationMgr(MMS_MALE, &AniMgrs[MMS_MALE]);
	SetAnimationMgr(MMS_FEMALE, &AniMgrs[MMS_FEMALE]);

	ret = MGetMatchItemDescMgr()->ReadXml(g_pFileSystem, "system/zitem.xml");
	if (!ret)
	{
		MGetMatchServer()->LogF(MMatchServer::LOG_ALL, "Falied to load zitems!");
		return false;
	}

	Maps.emplace("Mansion", RBspObject());
	ret = Maps["Mansion"].Open("maps/Mansion/Mansion.rs", RBspObject::ROF_RUNTIME, nullptr, nullptr, true);
	if (!ret)
	{
		MGetMatchServer()->LogF(MMatchServer::LOG_ALL, "Falied to load Mansion!");
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

	for (int i = 0; i < nCnt; i++) {

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

RBspObject * LagCompManager::GetBspObject(const char * MapName)
{
	auto it = Maps.find(MapName);
	if (it == Maps.end())
		return nullptr;

	return &it->second;
}