#include "stdafx.h"
#include <stdio.h>
#include <math.h>
#include <tchar.h>

#include "MXml.h"

#include "RealSpace2.h"
#include "RMesh.h"
#include "RMeshMgr.h"

#include "MDebug.h"

#include "RAnimationMgr.h"
#include "RVisualmeshMgr.h"

#include "MZFileSystem.h"
#include "fileinfo.h"

#include "RShaderMgr.h"

#ifndef _PUBLISH

#define __BP(i,n)	MBeginProfile(i,n);
#define __EP(i)		MEndProfile(i);

#else

#define __BP(i,n) ;
#define __EP(i) ;

#endif

_NAMESPACE_REALSPACE2_BEGIN

////////////////////////////////////////////////////////////////////////////////
//

bool RMesh::ReadXmlElement(MXmlElement* PNode,char* Path)
{
	char NodeName[256];
	char IDName[256];
	char FileName[256];
	char PathFileName[256];
	char MotionTypeID[256];
	char MotionLoopTypeID[256];
	char SoundFileName[256];
	char GameMotion[256];
	char PathSoundFileName[256];
//	char szSize[256];

	RMesh* pMesh = NULL;

	int nMTypeID = -1;
	bool bSoundMap = false;
	AnimationLoopType MLoopType = RAniLoopType_Loop;

	int nCnt = PNode->GetChildNodeCount();

	MXmlElement Node;

	for (int i=0; i<nCnt; i++) {

		Node = PNode->GetChildNode(i);

		Node.GetTagName(NodeName);

		if (NodeName[0] == '#') continue;

		if (strcmp(NodeName, "AddBaseModel")==0) {

			Node.GetAttribute(IDName, "name");
			Node.GetAttribute(FileName, "filename");

			if(Path) {
				strcpy_safe(PathFileName,Path);
				strcat(PathFileName,FileName);
			}
			else {
				strcpy_safe(PathFileName,FileName);
			}

			ReadElu(PathFileName);

			m_isMultiAniSet = true;
		}
		else if(strcmp(NodeName, "CharacterModel")==0) {
			m_isCharacterMesh = true;
		}
		else if(strcmp(NodeName, "NPCModel")==0) {
			m_isNPCMesh = true;
		}
		else if(strcmp(NodeName, "MakeAnimationMap")==0) {

//			Node.GetAttribute(szSize,"size");//나중에 필요하면 살리자.. 지금은 그때그때 바꾸는것이 더 귀찮다..
			m_ani_mgr.MakeListMap( (int)eq_weapon_end );//최대모션타잎만큼 만들어준다..
		}
		else if(strcmp(NodeName, "AddParts")==0) {
			extern bool IsDynamicResourceLoad();
			if(IsDynamicResourceLoad())
				continue;

			if(RMesh::m_parts_mesh_loading_skip==0) {
			
			Node.GetAttribute(IDName, "name");
			Node.GetAttribute(FileName, "filename");

			if(!m_parts_mgr) {
				m_parts_mgr = new RMeshMgr;
			}

			if(Path[0]) {
				strcpy_safe(PathFileName,Path);
				strcat(PathFileName,FileName);
			}
			else 
				strcpy_safe(PathFileName,FileName);

			int ret = m_parts_mgr->Add(PathFileName);

			if(ret==-1) {
				return false;
			}

			}
		}
		else if(strcmp(NodeName, "AddParticle")==0) {

			Node.GetAttribute(IDName, "name");
			Node.GetAttribute(FileName, "dummy_name");

			m_ParticleLinkInfo.Set(IDName,FileName);
		
		}
		else if(strcmp(NodeName, "AddAnimation")==0) {

			SoundFileName[0] = NULL;
			Node.GetAttribute(IDName, "name");
			Node.GetAttribute(FileName, "filename");
			Node.GetAttribute(MotionTypeID, "motion_type");
			Node.GetAttribute(MotionLoopTypeID,"motion_loop_type");
			Node.GetAttribute(SoundFileName,"sound");
			Node.GetAttribute(&bSoundMap, "soundmap", false);
			Node.GetAttribute(GameMotion,"gm");

			if(strcmp(MotionLoopTypeID,"normal")==0) {
				MLoopType = RAniLoopType_Normal;
			}
			else if(strcmp(MotionLoopTypeID,"loop")==0) {
				MLoopType = RAniLoopType_Loop;
			}
			else if(strcmp(MotionLoopTypeID,"onceidle")==0) {
				MLoopType = RAniLoopType_OnceIdle;
			}
			else if(strcmp(MotionLoopTypeID,"lastframe")==0) {
				MLoopType = RAniLoopType_HoldLastFrame;
			}
			else if(strcmp(MotionLoopTypeID,"onceLowerbody")==0) {
				MLoopType = RAniLoopType_OnceLowerBody;
			}

			//MLog("Name: %s, filename: %s, motion_type %s, motion_loop_type %s\n", IDName, FileName, MotionTypeID, MotionLoopTypeID);

			int nGameMotion = atoi(GameMotion);

			if( GetToolMesh() ) // 툴에서 사용되는거라면 무조건 바로 로딩~
				nGameMotion = 0;

			nMTypeID = atoi(MotionTypeID);

			if(Path[0]) {
				strcpy_safe(PathFileName,Path);
				strcat(PathFileName,FileName);
			}
			else 
				strcpy_safe(PathFileName,FileName);

			RAnimation* pAni = NULL;

			if(nGameMotion==1) { // 게임 모션은 나중에 로딩
				pAni = m_ani_mgr.AddGameLoad(IDName,PathFileName,-1,nMTypeID);
			}
			else {
				pAni = m_ani_mgr.Add(IDName,PathFileName,-1,nMTypeID);
			}

			if(pAni) {

				pAni->SetAnimationLoopType( MLoopType );

				if(SoundFileName[0]==NULL) {//사운드가 등록되지 않았으면 에니메이션 파일 이름이 기본 사운드이름..
					int len = (int) strlen(FileName);
					strncpy(SoundFileName,FileName,len-8);//.elu.ani 생략...
					SoundFileName[len-8] = NULL;

					strcpy_safe(PathSoundFileName,"/sound/effect/");
					strcat(PathSoundFileName,SoundFileName);
				}

				pAni->SetSoundFileName(SoundFileName);
				pAni->SetSoundRelatedToMap(bSoundMap);
			}
		}
	}

	return true;
}

bool RMesh::ReadXml(char* filename)
{
	MXmlDocument	XmlDoc;
	MXmlElement		PNode,Node;

	XmlDoc.Create();

	char Path[256];
	Path[0] = NULL;

	string BackupName = filename;//입력 받은것이 내부에서 바뀐다.

	GetPath(filename,Path);

	//<--------

//	if( !XmlDoc.LoadFromFile(filename) ) 
//		return false;

	char *buffer;
	MZFile mzf;

	if(g_pFileSystem) {
		if(!mzf.Open(filename,g_pFileSystem)) {
			if(!mzf.Open(filename))
				return false;
		}
	} else {
		if(!mzf.Open(filename))
			return false;
	}

	buffer = new char[mzf.GetLength()+1];
	buffer[mzf.GetLength()] = 0;

	mzf.Read(buffer,mzf.GetLength());

	if(!XmlDoc.LoadFromMemory(buffer))
		return false;

	delete[] buffer;

	mzf.Close();

	//-------->

	PNode = XmlDoc.GetDocumentElement();

	if(ReadXmlElement(&PNode,Path)==false) {
		XmlDoc.Destroy();
		return false;
	}

	XmlDoc.Destroy();

	SetFileName((char*)BackupName.c_str());

	return true;	
}

bool RMesh::SaveXml(char* fname)
{
	return true;
}

static void ConvertOldFaceInfo(RFaceInfo* pFInfo,RFaceInfoOld* pOldFInfo,int cnt)
{
	for(int i=0;i<cnt;i++) { 

		memcpy( pFInfo[i].m_point_index,pOldFInfo[i].m_point_index,sizeof(int)*3 );
		memcpy( pFInfo[i].m_point_tex,pOldFInfo[i].m_point_tex,sizeof(D3DXVECTOR3)*3 );

		pFInfo[i].m_mtrl_id = pOldFInfo[i].m_mtrl_id;
		pFInfo[i].m_sg_id = 0;

	}
}

static bool e_sort_str(RMeshNode* _a,RMeshNode* _b) {
	if( _a->m_Name < _a->m_Name )
		return true;
	return false;
}

static int CheckEf(const char* str)
{
	if(!str) return 0;

	int _strlen = (int)strlen(str);

	for(int i=0;i<_strlen-3;i++) {

		if( str[i] == '_' ) {
			if(str[i+1] == 'e') {
				if(str[i+2]=='f') {
					return 1;
				}
			}
		}
	}
	return 0;
}

static int CheckEfAlign(const char* str,int& ef,int& align)
{
	if(!str || str[0]==0)	return -1;

	static char word1[] = "ef_algn";
	static char word2[] = "algn";

	int str_len = (int)strlen(str);

	int word1_len = (int)strlen(word1);
	int word2_len = (int)strlen(word2);

	char c;

	ef = 0;
	align = 0;

	// 그냥 풀어쓰자..

	// algn0
	// ef_algn0
	bool bCheck = true;

	for(int i=0;i<str_len;i++) {
		
		c = str[i];
		if(c == word1[0]) {
			bCheck = true;
			for(int j=1;j<word1_len;j++) {
				if(str[i+j]!=word1[j]) {
					bCheck = false;
				}
			}
			if(bCheck==true) {
				ef = 1;
				if(str[i+word1_len]=='0')
					align = 1;
				else if(str[i+word1_len]=='1')
					align = 2;
			}
		} 
		else if(c==word2[0]) {
			bCheck = true;
			for(int j=1;j<word2_len;j++) {
				if(str[i+j]!=word2[j]) {
					bCheck = false;
				}
			}
			if(bCheck==true) {
				if(str[i+word2_len]=='0')
					align = 1;
				else if(str[i+word2_len]=='1')
					align = 2;
			}
		}
	}
	return -1;
}

void RMesh::CheckNameToType(RMeshNode* pMeshNode)
{
	pMeshNode->m_PartsType = eq_parts_etc;
//	pMeshNode->m_WeaponMotionType = eq_weapon_etc;

	pMeshNode->m_isDummy = false;
	pMeshNode->m_isWeaponMesh = false;

	auto* pName = pMeshNode->GetName();

#define NCMPNAME(name,n)	(_strnicmp(pName,name,n)==0)
#define CMPNAME(name)		(_stricmp(pName,name)==0)


	if(NCMPNAME("Bip",3)) {		
		pMeshNode->m_isDummyMesh = true;
//		pMeshNode->ConnectToNameID();//이름을 ID 로 바꾼다..
	}
	else if(NCMPNAME("Bone",4))		pMeshNode->m_isDummyMesh = true;
	else if(NCMPNAME("Dummy",5)) {
		pMeshNode->m_isDummyMesh = true;
		pMeshNode->m_isDummy = true;
	}
	else if( NCMPNAME("eq_wr",5) || NCMPNAME("eq_wl",5) || NCMPNAME("eq_wd",5) ) {// 전부 소문자화~
		// 오른손 무기 더미 종류
		if( NCMPNAME("eq_wd_katana",12) ) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_right_katana;
		}
		else if( NCMPNAME("eq_wl_pistol",12) ) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_left_pistol;
		}
		else if( NCMPNAME("eq_wr_pistol",12) ) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_right_pistol;
		}
		else if( NCMPNAME("eq_wd_pistol",12 )) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_right_pistol;
		}
		else if( NCMPNAME("eq_wl_smg",9) ) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_left_smg;
		}
		else if( NCMPNAME("eq_wr_smg",9) ) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_right_smg;
		}
		else if( NCMPNAME("eq_wd_smg",9) ) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_right_smg;
		}
		else if( NCMPNAME("eq_wd_shotgun",13) ) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_right_shotgun;
		}
		else if( NCMPNAME("eq_wd_rifle",11) ) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_right_rifle;
		}
		else if( NCMPNAME("eq_wd_grenade",13) ) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_right_grenade;
		}
		else if( NCMPNAME("eq_wd_item",10) ) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_right_item;
		}
		else if( NCMPNAME("eq_wl_dagger",12) ) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_left_dagger;
		}
		else if( NCMPNAME("eq_wr_dagger",12) ) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_right_dagger;
		}
		else if( NCMPNAME("eq_wd_medikit",13) ) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_right_item;
		}
		else if( NCMPNAME("eq_wd_rl",8) ) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_right_rlauncher;
		}
		else if( NCMPNAME("eq_wd_sword",11) ) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_right_sword;
		}
		else if( NCMPNAME("eq_wr_blade",11) ) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_right_blade;
		}
		else if( NCMPNAME("eq_wl_blade",11) ) {
			pMeshNode->m_isWeaponMesh = true;
			pMeshNode->m_PartsType = eq_parts_left_blade;
		}
	} 
	else if(NCMPNAME("eq_",3) ) {
		// 뒤쪽은 고유한 이름..
		if(NCMPNAME("eq_head",7) )	{
			pMeshNode->m_PartsType = eq_parts_head;
			pMeshNode->m_AlphaSortValue = 1.f;//알파값이 있는경우 소팅우선순위
		}
		else if(NCMPNAME("eq_face",7) ) {
			pMeshNode->m_PartsType = eq_parts_face;
			pMeshNode->m_AlphaSortValue = 2.f;
		}
		else if(NCMPNAME("eq_chest",8) ) {	
			pMeshNode->m_PartsType = eq_parts_chest;
//			pMeshNode->m_isCollisionMesh = true;
			pMeshNode->m_AlphaSortValue = 3.f;
		}
		else if(NCMPNAME("eq_hands",8) ) {
			pMeshNode->m_PartsType = eq_parts_hands;
			pMeshNode->m_AlphaSortValue = 4.f;
		}
		else if(NCMPNAME("eq_legs",7) )	{
			pMeshNode->m_PartsType = eq_parts_legs;
			pMeshNode->m_AlphaSortValue = 5.f;
		}
		else if(NCMPNAME("eq_feet",7) )	{
			pMeshNode->m_PartsType = eq_parts_feet;
			pMeshNode->m_AlphaSortValue = 6.f;
		}
		else if(NCMPNAME("eq_sunglass",11) )	{
			pMeshNode->m_PartsType = eq_parts_sunglass;
			pMeshNode->m_AlphaSortValue = 0.5f;
		}
	}
	else if(NCMPNAME("collision_",10)) {	// 충돌이나 picking 용 모델
		pMeshNode->m_isCollisionMesh = true;
		pMeshNode->m_isDummyMesh = true;
	}
/*
		Bip01
		Bip01 Head
		Bip01 HeadNub
		Bip01 L Calf
		Bip01 L Clavicle
		Bip01 L Finger0
		Bip01 L FingerNub
		Bip01 L Foot
		Bip01 L ForeArm
		Bip01 L Hand
		Bip01 L Thigh
		Bip01 L Toe0
		Bip01 L Toe0Nub
		Bip01 L UpperArm
		Bip01 Neck
		Bip01 Pelvis
		Bip01 R Calf
		Bip01 R Clavicle
		Bip01 R Finger0
		Bip01 R FingerNub
		Bip01 R Foot
		Bip01 R ForeArm
		Bip01 R Hand
		Bip01 R Thigh
		Bip01 R Toe0
		Bip01 R Toe0Nub
		Bip01 R UpperArm
		Bip01 Spine
		Bip01 Spine1
		Bip01 Spine2
*/

	if(NCMPNAME("lastmodel",9)) {
		pMeshNode->m_isLastModel = true;
	}

	if(NCMPNAME("deffect",7)) {
		pMeshNode->m_isDummyMesh = true;
		pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_Effect;
	}


	if(NCMPNAME("Bip",3)) {

		// 하체노드 표시..

		if(NCMPNAME("Bip01 L",7)) {

			if(NCMPNAME("Bip01 L Calf",12)) {
				pMeshNode->m_CutPartsType = cut_parts_lower_body;
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_LCalf;
			}
			else if(NCMPNAME("Bip01 L Clavile",15)) {
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_LClavicle;
			}
			else if(NCMPNAME("Bip01 L Finger0",15)) {
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_LFinger0;
			}
			else if(NCMPNAME("Bip01 L FingerNub",17)) {
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_LFingerNub;
			}
			else if(NCMPNAME("Bip01 L Foot",12)) {
				pMeshNode->m_CutPartsType = cut_parts_lower_body;
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_LFoot;
			}
			else if(NCMPNAME("Bip01 L ForeArm",15)) {
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_LForeArm;
			}
			else if(NCMPNAME("Bip01 L Hand",12)) {
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_LHand;
			}
			else if(NCMPNAME("Bip01 L Thigh",13)) {	
				pMeshNode->m_CutPartsType = cut_parts_lower_body;
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_LThigh;
			}
			else if(NCMPNAME("Bip01 L Toe0Nub",15)) {
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_LToe0Nub;
			}
			else if(NCMPNAME("Bip01 L Toe0",12)) {
				pMeshNode->m_CutPartsType = cut_parts_lower_body;
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_LToe0;
			}
			else if(NCMPNAME("Bip01 L UpperArm",16)) {
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_LUpperArm;
			}
		}
		else if(NCMPNAME("Bip01 R",7)) {

			if(NCMPNAME("Bip01 R Calf",12)) {	
				pMeshNode->m_CutPartsType = cut_parts_lower_body;
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_RCalf;
			}
			else if(NCMPNAME("Bip01 R Clavile",15)) {
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_RClavicle;
			}
			else if(NCMPNAME("Bip01 R Finger0",15)) {
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_RFinger0;
			}
			else if(NCMPNAME("Bip01 R FingerNub",17)) {
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_RFingerNub;
			}
			else if(NCMPNAME("Bip01 R Foot",12)) {
				pMeshNode->m_CutPartsType = cut_parts_lower_body;
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_RFoot;
			}
			else if(NCMPNAME("Bip01 R ForeArm",15)) {
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_RForeArm;
			}
			else if(NCMPNAME("Bip01 R Hand",12)) {
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_RHand;
			}
			else if(NCMPNAME("Bip01 R Thigh",13))	{
				pMeshNode->m_CutPartsType = cut_parts_lower_body;
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_RThigh;
			}
			else if(NCMPNAME("Bip01 R Toe0Nub",15)) {
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_RToe0Nub;
			}
			else if(NCMPNAME("Bip01 R Toe0",12)) {
				pMeshNode->m_CutPartsType = cut_parts_lower_body;
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_RToe0;
			}
			else if(NCMPNAME("Bip01 R UpperArm",16)) {
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_RUpperArm;
			}
		}
		else {

			if(CMPNAME("Bip01")) {
				pMeshNode->m_CutPartsType	= cut_parts_lower_body;
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_Root;
			}
			else if(CMPNAME("Bip01 Head")) {
				pMeshNode->m_LookAtParts = lookat_parts_head;
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_Head;
			}
			else if(CMPNAME("Bip01 HeadNub")) {
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_HeadNub;
			}
			else if(CMPNAME("Bip01 Neck")) {
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_Neck;
			}
			else if(CMPNAME("Bip01 Pelvis")) {
				pMeshNode->m_CutPartsType = cut_parts_lower_body;
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_Pelvis;
			}
			else if(CMPNAME("Bip01 Spine"))	{
				pMeshNode->m_CutPartsType = cut_parts_lower_body;
				pMeshNode->m_LookAtParts = lookat_parts_spine;
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_Spine;
			}
			else if(CMPNAME("Bip01 Spine1")) {
				pMeshNode->m_LookAtParts = lookat_parts_spine1;
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_Spine1;
			}
			else if(CMPNAME("Bip01 Spine2")) {
				pMeshNode->m_LookAtParts = lookat_parts_spine2;
				pMeshNode->m_PartsPosInfoType = eq_parts_pos_info_Spine2;
			}
		}
	}

	if(NCMPNAME("Bone",4)) {
		pMeshNode->m_CutPartsType = cut_parts_lower_body;
	}

	if(NCMPNAME("Dummy",5)) {
		pMeshNode->m_CutPartsType = cut_parts_lower_body;
	}

	int ef = 0;
	int align = 0;

	//시간날때 맵의 오브젝트 프로퍼티로 옮기자~~

	CheckEfAlign(pMeshNode->GetName(),ef,align);

	ef = CheckEf(pMeshNode->GetName());

	if(ef==1) {
		m_bEffectSort = true;
		m_LitVertexModel = true;
	}

	pMeshNode->m_nAlign = align;

/*
	test..

	CheckEfAlign("algn0_test1",ef,align);
	CheckEfAlign("algn1_test1",ef,align);

	CheckEfAlign("ef_algn0_test1",ef,align);
	CheckEfAlign("ef_algn1_test2",ef,align);

	CheckEfAlign("map1_algn0_test2",ef,align);
	CheckEfAlign("map2_algn1_test2",ef,align);

	CheckEfAlign("map1_ef_algn0_test2",ef,align);
	CheckEfAlign("map2_ef_algn1_test2",ef,align);
*/
	// 무기의 발사 위치 추가

//	"muzzle_flash"		//총구화염
//	"empty_cartridge01"	//일반탄피
//	"empty_cartridge02"	//산탄탄피

	if( NCMPNAME("muzzle_flash",12) ) {
		pMeshNode->m_WeaponDummyType = weapon_dummy_muzzle_flash;
	}
	else if( NCMPNAME("empty_cartridge01",17) ){
		pMeshNode->m_WeaponDummyType = weapon_dummy_cartridge01;
	}
	else if( NCMPNAME("empty_cartridge02",17) ){
		pMeshNode->m_WeaponDummyType = weapon_dummy_cartridge02;
	}

#undef NCMPNAME

}

bool RMesh::ReadElu(const char* fname)
{
#define MZF_READ(x,y) { if(!mzf.Read((x),(y))) return false; }

	__BP(2009,"RMesh::ReadElu");

	char Path[256];
	char Name[256];

	Path[0] = NULL;
	Name[0] = NULL;

	// 이펙트 모델이라면 자동으로 옵션켠다.

	GetPath(fname,Path);

	int len = strlen(Path);

	if(strncmp(&fname[len],"ef_",3)==0) {
		m_bEffectSort = true;
		m_LitVertexModel = true;
	}
	else {//이펙트가 아니라면 텍스쳐 해상도 옵션의 영향을 받음.
		m_mtrl_list_ex.SetObjectTexture(true);
	}

	SetFileName(fname);

	m_data_num = 0;

//	char *buffer;
	MZFile mzf;

	if(g_pFileSystem) {
		if(!mzf.Open(fname,g_pFileSystem)) {
			if(!mzf.Open(fname)) {
				mlog("----------> in zip ( %s ) file not found!! \n ", fname );
				return false;
			}
		}
	} else {
		if(!mzf.Open(fname)) {
			mlog("----------> %s file not found!! \n ", fname );
			return false;
		}
	}

//	buffer = new char[mzf.GetLength()+1];
//	buffer[mzf.GetLength()] = 0;

//	mzf.Read(buffer,mzf.GetLength());

//	if(!XmlDoc.LoadFromMemory(buffer))
//		return false;

//	delete[] buffer;

//	FILE *fp;
//	fp  = fopen(fname, "rb");

//	if(!fp) return false;

	ex_hd_t t_hd;

	MZF_READ(&t_hd,sizeof(ex_hd_t) );
/*
	if(t_hd.ver != EXPORTER_VER) {
		mlog("%s elu file 버젼이 틀림.\n",fname);
		return false;
	}
*/
	if(t_hd.sig != EXPORTER_SIG) {
		mlog("%s elu file 파일 식별 실패.\n",fname);
		return false;
	}
	
	int i,j,k;
	// sub mtrl 까지 포함한 갯수
	for(i=0;i<t_hd.mtrl_num;i++) {

		RMtrl* node = new RMtrl;

		MZF_READ(&node->m_mtrl_id    ,4 );
		MZF_READ(&node->m_sub_mtrl_id,4 );

		MZF_READ(&node->m_ambient ,sizeof(D3DXCOLOR) );
		MZF_READ(&node->m_diffuse ,sizeof(D3DXCOLOR) );
		MZF_READ(&node->m_specular,sizeof(D3DXCOLOR) );

		MZF_READ(&node->m_power,4 );

		node->m_power *= 100.f;

		if(t_hd.ver <= EXPORTER_MESH_VER3)
			if(node->m_power == 2000.f)
				node->m_power = 0.f;

//		node->m_power = 80.f;

		MZF_READ(&node->m_sub_mtrl_num,4 );

		if(t_hd.ver< EXPORTER_MESH_VER7) {
			MZF_READ(&node->m_name    ,MAX_NAME_LEN );
			MZF_READ(&node->m_opa_name,MAX_NAME_LEN );
		}
		else {
			MZF_READ(&node->m_name    ,MAX_PATH_NAME_LEN );
			MZF_READ(&node->m_opa_name,MAX_PATH_NAME_LEN );
		}

		//MLog("Mtrl name %s\n", node->m_name);

		if(t_hd.ver > EXPORTER_MESH_VER2) {//ver3 부터
			int twoside=0;
			MZF_READ(&twoside,sizeof(int) );
			node->m_bTwoSided = twoside ? true : false;
		}

		if(t_hd.ver > EXPORTER_MESH_VER4) {
			int additive = 0;
			MZF_READ(&additive,sizeof(int) );
			node->m_bAdditive = additive ? true : false;
		}

		if(t_hd.ver > EXPORTER_MESH_VER7 )//ver8 부터
		{
			int alpha_test = 0;
			MZF_READ(&alpha_test,sizeof(int) );
			node->m_bAlphaTestMap = alpha_test ? true : false;
			node->m_nAlphaTestValue = alpha_test;
		}
		
		if( node->m_name[0] ) {

			int	 len = strlen(node->m_name);
			char _temp[5];

			strncpy(_temp,&node->m_name[len-4],4);

			_temp[4] = 0;

			if( _stricmp(_temp,".tga")==0 ) {
				node->m_bDiffuseMap = true;
			}

			if( node->m_opa_name[0] ) {
				node->m_bAlphaMap	= true;
				node->m_bDiffuseMap = false;
			}
		}

		if( node->m_bAlphaTestMap ) {
			node->m_bAlphaMap	= false;
			node->m_bDiffuseMap = false;
		}

		node->CheckAniTexture();

		m_mtrl_list_ex.Add(node);
	}

	bool bNeedScaleMat = false;
	D3DXMATRIX smat;

	for(i=0;i<t_hd.mesh_num;i++) {

		bNeedScaleMat = false;

		RMeshNode* pMeshNode = new RMeshNode;

//		memset( pMeshNode, 0, sizeof(RMeshNode));
//		pMeshNode->m_mtrl_id = -1;
		D3DXMatrixIdentity(&pMeshNode->m_mat_base);

		pMeshNode->m_id = m_data_num;//last id
		pMeshNode->m_pParentMesh = this;
		pMeshNode->m_pBaseMesh = this;

		MZF_READ(Name  ,MAX_NAME_LEN );
//		MZF_READ(pMeshNode->m_Name  ,MAX_NAME_LEN );
		MZF_READ(pMeshNode->m_Parent,MAX_NAME_LEN );
		MZF_READ(&pMeshNode->m_mat_base,sizeof(D3DXMATRIX) );//mat

		pMeshNode->SetName( Name );

		//MLog("Node name: %s, parent %s\n", pMeshNode->GetName(), pMeshNode->m_Parent);

		pMeshNode->m_mat_ref = pMeshNode->m_mat_base;
		D3DXMatrixInverse( &pMeshNode->m_mat_ref_inv , 0, &pMeshNode->m_mat_ref );
		
		if(t_hd.ver >= EXPORTER_MESH_VER2) {
			MZF_READ(&pMeshNode->m_ap_scale,sizeof(D3DXVECTOR3) );//mat
		}
		else  {
			pMeshNode->m_ap_scale.x = 1.f;
			pMeshNode->m_ap_scale.y = 1.f;
			pMeshNode->m_ap_scale.z = 1.f;
		}

		///////////////////////////////////////////////

		if(t_hd.ver >= EXPORTER_MESH_VER4) {

			MZF_READ(&pMeshNode->m_axis_rot,sizeof(D3DXVECTOR3) );
			MZF_READ(&pMeshNode->m_axis_rot_angle,sizeof(float) );

			MZF_READ(&pMeshNode->m_axis_scale,sizeof(D3DXVECTOR3) );
			MZF_READ(&pMeshNode->m_axis_scale_angle,sizeof(float) );

			MZF_READ(&pMeshNode->m_mat_etc,sizeof(D3DXMATRIX) );//mat

			D3DXMATRIX scalemat;
			D3DXMATRIX scalepivot;
			D3DXMATRIX scalepivotinv;
			D3DXMATRIX flipmat;

			D3DXMatrixScaling( &scalemat, pMeshNode->m_ap_scale.x, pMeshNode->m_ap_scale.y, pMeshNode->m_ap_scale.z );
			D3DXMatrixRotationAxis( &scalepivot, &pMeshNode->m_axis_scale, pMeshNode->m_axis_scale_angle );
			D3DXMatrixInverse( &scalepivotinv, NULL, &scalepivot );

			D3DXMatrixIdentity(&flipmat);

			pMeshNode->m_mat_flip = flipmat;

//			result_mat2 = scalepivotinv * scalemat * scalepivot;
			pMeshNode->m_mat_etc = scalepivotinv * scalemat * scalepivot;

		}
		else {

			D3DXMatrixIdentity(&pMeshNode->m_mat_etc);
			D3DXMatrixIdentity(&pMeshNode->m_mat_flip);
		}

		memcpy(&pMeshNode->m_mat_local,&pMeshNode->m_mat_base,sizeof(D3DXMATRIX));

		pMeshNode->m_mat_result = pMeshNode->m_mat_base;

		D3DXMatrixScaling(&pMeshNode->m_mat_scale, pMeshNode->m_ap_scale.x, pMeshNode->m_ap_scale.y, pMeshNode->m_ap_scale.z);

		RMatInv(pMeshNode->m_mat_inv,pMeshNode->m_mat_local);

		/*if (pMeshNode->m_Name == "Bip01 Neck")
		{
			DMLog("Node base:\n");

			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					DMLog("%f ", pMeshNode->m_mat_base(i, j));
				}

				DMLog("\n");
			}

			DMLog("Node inv:\n");

			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					DMLog("%f ", pMeshNode->m_mat_inv(i, j));
				}

				DMLog("\n");
			}
		}*/

		CheckNameToType(pMeshNode);

		MZF_READ(&pMeshNode->m_point_num,4 );

		if(pMeshNode->m_point_num) {
		
			pMeshNode->m_point_list = new D3DXVECTOR3[pMeshNode->m_point_num];
			memset(pMeshNode->m_point_list,0,pMeshNode->m_point_num * sizeof(D3DXVECTOR3));

			MZF_READ(pMeshNode->m_point_list,sizeof(D3DXVECTOR3)*pMeshNode->m_point_num);

			pMeshNode->CalcLocalBBox();
		}

		//tex uv 까지 포함

		MZF_READ(&pMeshNode->m_face_num,4 );

		if(pMeshNode->m_face_num) {

			pMeshNode->m_face_list = new RFaceInfo[pMeshNode->m_face_num];
			pMeshNode->m_face_normal_list = new RFaceNormalInfo[pMeshNode->m_face_num];

			memset(pMeshNode->m_face_list, 0, pMeshNode->m_face_num * sizeof(RFaceInfo));

			if( t_hd.ver >= EXPORTER_MESH_VER6 ) {//ver 6

				MZF_READ(pMeshNode->m_face_list,sizeof(RFaceInfo)*pMeshNode->m_face_num);
				MZF_READ(pMeshNode->m_face_normal_list,sizeof(RFaceNormalInfo)*pMeshNode->m_face_num);

			}
			else if(t_hd.ver > EXPORTER_MESH_VER2) {//ver3 부터

				MZF_READ(pMeshNode->m_face_list,sizeof(RFaceInfo)*pMeshNode->m_face_num);
			}
			else {									//ver3 이하

				RFaceInfoOld* pInfo = new RFaceInfoOld[pMeshNode->m_face_num];
				MZF_READ(pInfo,sizeof(RFaceInfoOld)*pMeshNode->m_face_num);

				ConvertOldFaceInfo(pMeshNode->m_face_list,pInfo,pMeshNode->m_face_num);

				delete[] pInfo;
			}
		}

		if( t_hd.ver >= EXPORTER_MESH_VER6 ) {
		
			MZF_READ(&pMeshNode->m_point_color_num,4 );

			if(pMeshNode->m_point_color_num) {
				pMeshNode->m_point_color_list = new D3DXVECTOR3 [pMeshNode->m_point_color_num];
				MZF_READ(pMeshNode->m_point_color_list,sizeof(D3DXVECTOR3)*pMeshNode->m_point_color_num);
			}
		}

		if( (pMeshNode->m_point_num==0) || (pMeshNode->m_face_num==0) ) {
			pMeshNode->m_isDummyMesh = true;
		}

		//////////////////////////////////////////////
		
		MZF_READ(&pMeshNode->m_mtrl_id,4 );

		MZF_READ(&pMeshNode->m_physique_num,4 );

		if(pMeshNode->m_physique_num) {
			
			m_isPhysiqueMesh = true;

			pMeshNode->m_physique = new RPhysiqueInfo[pMeshNode->m_physique_num];
			ZeroMemory(pMeshNode->m_physique,pMeshNode->m_physique_num * sizeof(RPhysiqueInfo));

			for(int j=0;j<pMeshNode->m_physique_num;j++) {

				MZF_READ( &pMeshNode->m_physique[j],sizeof(RPhysiqueInfo) );
			}
		}

		D3DXPLANE	plane;
		D3DXVECTOR3	vv[3];

		if( t_hd.ver < EXPORTER_MESH_VER6 ) {

			if(pMeshNode->m_face_num) {
			
				for(int a=0;a<pMeshNode->m_face_num;a++) {

					vv[0] = pMeshNode->m_point_list[pMeshNode->m_face_list[a].m_point_index[0]];
					vv[1] = pMeshNode->m_point_list[pMeshNode->m_face_list[a].m_point_index[1]];
					vv[2] = pMeshNode->m_point_list[pMeshNode->m_face_list[a].m_point_index[2]];

					D3DXPlaneFromPoints(&plane,&vv[0],&vv[1],&vv[2]);
					D3DXPlaneNormalize(&plane,&plane);

					pMeshNode->m_face_normal_list[a].m_normal.x = plane.a;
					pMeshNode->m_face_normal_list[a].m_normal.y = plane.b;
					pMeshNode->m_face_normal_list[a].m_normal.z = plane.c;
				}
			}

			if(pMeshNode->m_point_num&&pMeshNode->m_point_num) 
			{
				D3DXVECTOR3* pPointNormal = new D3DXVECTOR3 [pMeshNode->m_point_num];
				memset(pPointNormal,0,sizeof(D3DXVECTOR3)*pMeshNode->m_point_num);
			

				for(k=0;k<pMeshNode->m_face_num;k++) {
					for(j=0;j<3;j++) {
						pPointNormal[ pMeshNode->m_face_list[k].m_point_index[j] ] =
							pPointNormal[pMeshNode->m_face_list[k].m_point_index[j]] + pMeshNode->m_face_normal_list[k].m_normal;
					}
				}

				for(k=0;k<pMeshNode->m_point_num;k++) {
					pPointNormal[k] = pPointNormal[k]/3.f;
					D3DXVec3Normalize(&pPointNormal[k],&pPointNormal[k]);
				}

				for(k=0;k<pMeshNode->m_face_num;k++) {
					for(j=0;j<3;j++) {
						pMeshNode->m_face_normal_list[k].m_pointnormal[j] = pPointNormal[ pMeshNode->m_face_list[k].m_point_index[j] ];
					}
				}

				delete [] pPointNormal;

			}
		}

		if(pMeshNode->m_point_color_num>0 && pMeshNode->m_PartsType == eq_parts_chest )
			pMeshNode->m_isClothMeshNode = true;

		m_list.PushBack(pMeshNode);
		
		m_data.push_back( pMeshNode );
		m_data_num++;

		if( MAX_MESH_NODE_TABLE != (int)m_data.capacity() )
		{
			mlog( "m_data number is not quite right..! \n" );
		}
	}

	// 안경(가면) - type 도 추가되어야 함..파츠용 더미..
	// 가방

	// 더미추가 - 모델은 무기처럼 찾아서 붙이고..

	//--------------------------------------------------

	if( m_isCharacterMesh ) {

		rmatrix _pbm;

		// 썬그라스 기본장비 위치..

		_pbm._11 = 0.f;
		_pbm._12 = 1.f;
		_pbm._13 = 0.f;
		_pbm._14 = 0.f;

		_pbm._21 = 0.1504f;
		_pbm._22 = -0.f;
		_pbm._23 = 0.9886f;
		_pbm._24 = 0.f;

		_pbm._31 = 0.9886f;
		_pbm._32 = 0.f;
		_pbm._33 = -0.1504f;
		_pbm._34 = 0.f;

		_pbm._41 = 9.0528f;
		_pbm._42 = 0.f;
		_pbm._43 = 9.8982f;
		_pbm._44 = 1.f;

		AddNode("eq_sunglass","Bip01 Head",_pbm);

	}

	//<------------------------------------------------

//	fclose (fp);
	mzf.Close();

	ConnectMatrix();

	///////////////////////////////////////////
	//mtrl list load

//	char Path[256];
//	GetPath(fname,Path);
//	m_mtrl_list_ex.Restore(RGetDevice(),Path);
	// map object 라면 마음대로 올리면 안됨...구분필요...

	if( m_is_map_object ) {
		ClearVoidMtrl();//연결안된 빈 mtrl 을 지운다...
	}

	if( m_mtrl_auto_load ) {
		m_mtrl_list_ex.Restore(RGetDevice(),Path);
	}

	ConnectMtrl();// Mtrl 연결..

	if(m_bEffectSort) {

		m_list.sort(e_sort_str);

		RMeshNodeHashList_Iter it_obj =  m_list.begin();

		int cnt = 0;

		while (it_obj !=  m_list.end()) {

			RMeshNode* pMeshNode = (*it_obj);

			m_data[cnt] = pMeshNode;
			pMeshNode->m_id = cnt;

			cnt++;
			it_obj++;
		}
	}

	CheckNodeAlphaMtrl();// 각노드 alpha mtrl 인가 체크..

	MakeAllNodeVertexBuffer();

//	mlog("elu file ( %s ) load... \n",fname);

	__EP(2009);

	m_isMeshLoaded = true;

	return true;
}

// old interface
// read + set 

bool RMesh::SaveElu(char* fname)
{
	return true;
}

// 더미용 모델연결.. ( 안경이나.. 가방등 )
// 각각의 부모는 에니메이션이 연결될때마다 base matrix 가 재설정 된다.
// 미리 local matrix 를 가지고 있다가 붙여주는 역할만하자..
// 메시가 read 될때만 쓰이도록 되어있다..아무곳에서나 사용하려면 부모연결등에 주의해서 다시작성...

bool RMesh::AddNode(char* name,char* pname,rmatrix& base_mat)
{
	RMeshNode* pMeshNode = new RMeshNode;

	pMeshNode->m_id = m_data_num;		// last id
	pMeshNode->m_pParentMesh = this;
	pMeshNode->m_pBaseMesh = this;

	pMeshNode->SetName(name);
	strcpy_safe(pMeshNode->m_Parent,pname);	// max 40

	pMeshNode->m_mat_base	= base_mat;	// 위치 잡아주고.. 출력된값으로..
	pMeshNode->m_mat_ref	= base_mat;
	pMeshNode->m_mat_result = base_mat;
	pMeshNode->m_mat_add	= base_mat; // 로컬을 유지하면서 나중에 사용..

	pMeshNode->m_isAddMeshNode = true;

	D3DXMatrixInverse( &pMeshNode->m_mat_ref_inv , 0, &pMeshNode->m_mat_ref );

	m_list.PushBack(pMeshNode);
	m_data.push_back( pMeshNode );
	m_data_num++;

	CheckNameToType(pMeshNode);

	return true;
}


// 제거시 자신을 부모로 가지는 오브젝트에 대해서고려~

bool RMesh::DelNode(RMeshNode* data) 
{
	return false;
}

void RMesh::MakeAllNodeVertexBuffer()
{
	// 버텍스 버퍼 만들기

	RMeshNode* pMeshNode=NULL;
	DWORD flag = 0;

	bool isShader = RIsSupportVS();

//	if( RIsSupportVS() )
	{
		for( int i = 0 ; i < m_data_num; ++i )
		{
			pMeshNode = m_data[i];

			if(pMeshNode->m_isDummyMesh) continue;
			if(!pMeshNode->m_point_num) continue;

			if( pMeshNode->m_physique && pMeshNode->m_point_color_num == 0 && isShader) // cloth 몸통이 아닌.. 모델들.. + 몬스터들
			{
				pMeshNode->MakeVSVertexBuffer();
			}

			if(pMeshNode->m_physique) {	// 몸통... color 버텍스를 포함한것들..
				// CreateVSVertexBuffer 는 인덱스버퍼는 생성하지 않고 여기서 생성한걸 사용한다...
				pMeshNode->MakeNodeBuffer( USE_VERTEX_SW );
			}
			else { // 일반 모델들.. 버텍스 에니의 경우 sw 만 사용.. ( 버텍스 에니 연결되기전에는 식별불가 )

				if(RIsHardwareTNL())	flag = USE_VERTEX_HW | USE_VERTEX_SW;
				else 					flag = USE_VERTEX_SW;
					
				pMeshNode->MakeNodeBuffer( flag ); // vertex ani 인경우만 soft 버퍼 생성..
			}
		}
	}
}

void RMesh::ConnectMatrix()
{
	// 파츠로 등록되는 모델들은 base model 에 대해서 연결해 줘야 한다...
	// ConnectPhysiqueParent 가 그 역할을 하고 있음..

	RMeshNode* pMeshNode=NULL;

	for(int i=0;i<m_data_num;i++) {

		pMeshNode = m_data[i];

		if(pMeshNode == NULL) continue;

//		if(pMeshNode->m_point_num) {
		if (pMeshNode->m_Parent[0] != 0) {
			CalcLocalMatrix(pMeshNode);
		}
//		}

		// 부모 연결
		int t_p_num,ret,j;
		
		if(pMeshNode->m_physique_num) {
		
			for(int q=0;q<pMeshNode->m_physique_num;q++) {

				t_p_num = pMeshNode->m_physique[q].m_num;

				// 최대 4개 지원
				if(t_p_num > MAX_PHYSIQUE_KEY)	
					t_p_num = MAX_PHYSIQUE_KEY;

				for(j=0;j<t_p_num;j++) {

					ret = _FindMeshId(&pMeshNode->m_physique[q].m_parent_name[j][0]);//로딩시만~

					if(ret != -1) {

						pMeshNode->m_physique[q].m_parent_id[j] = ret;
					}
					else {
						//mlog("ReadElu() : physique %d %d %s not found !!\n",i,j,&pMeshNode->m_physique[q].m_parent_name[j][0]);
					}
				}
			}
/*
			if( pMeshNode->m_isDummyMesh==false )// 스킨정보가 있는 상태에서 모델 노드라면 BaseBoneMatrix 를 만들어준다.
			{
				// 사용하는 본의 갯수 - 에니메이션이 바뀌더라도 바뀌지 않는다.
				
				int k = t_data_use_cnt;

				pMeshNode->m_BoneBaseMatrix = new RBoneBaseMatrix[t_data_use_cnt];

				int pos = 0;

				for(int i=0;i<m_data_num;i++)
				{
					if(t_data[i] != -1)
					{
						pMeshNode->m_BoneBaseMatrix[pos].id = i;
						pMeshNode->m_BoneBaseMatrix[pos].mat = pMeshNode->m_mat_ref * m_data[i]->m_mat_ref_inv;
					}
				}
			}
			파츠 때문에 복잡해진다.
*/
		}

//		if(pMeshNode->m_physique_num||p1||p2||p3)
//			mlog("%s physique vertex %d , p1 %d , p2 %d , p3 %d\n",pMeshNode->m_Name,pMeshNode->m_physique_num,p1,p2,p3);

		////////////////////////////////////////////////////

		if( m_isNPCMesh ) // npc 의 손에든 무기 구분하기..
		{ 
			int id = _FindMeshId(pMeshNode->m_Parent);

			RMeshNode* pPN = NULL;

			if(id != -1) 
				pPN = m_data[id];

			if( pPN ) {

				if(!_stricmp( pPN->GetName() , "Bip01 L Hand") || !_stricmp( pPN->GetName() , "Bip01 R Hand"))
					pMeshNode->m_bNpcWeaponMeshNode = true;
			}
		}

	}
}

bool RMesh::CalcLocalMatrix(RMeshNode* pNode)
{
	if(!pNode) 
		return false;

	D3DXMATRIX dest,inv;

	D3DXMatrixIdentity(&inv);

	RMeshNode* pPN = NULL;

	int id = _FindMeshId(pNode->m_Parent);

	if(id != -1) {

		pPN = m_data[id];

		if(pPN) {
			pNode->m_pParent = pPN;

//			RMatInv(inv,pPN->m_mat_base);								// 부모 인버스 등록
			D3DXMatrixInverse(&inv,NULL,&pPN->m_mat_base);
/*
			if(pPN->m_Parent[0]==0) {
				memcpy(&pNode->m_mat_parent_inv,&inv,sizeof(D3DXMATRIX));
			}
			else {
				D3DXMatrixMultiply(&inv,&pPN->m_mat_parent_inv,&inv);
				memcpy(&pNode->m_mat_parent_inv,&inv,sizeof(D3DXMATRIX));//부모의 부모에대한 상대 인버스값
			}
*/
//			if(pNode->m_WeaponDummyType != weapon_dummy_etc) { int k=0; }

			D3DXMatrixMultiply(&pNode->m_mat_local,&pNode->m_mat_base,&inv);

			return true;
		}
	}

	if(pNode->m_Parent[0])
		mlog("%s 노드의 %s 부모 못 찾음\n",pNode->m_Name,pNode->m_Parent);

	return false;
}

bool RMesh::ConnectPhysiqueParent(RMeshNode* pNode)
{
	// 부모 연결
	int p_num,ret,j;

	if (!pNode)		return false;

	if(pNode->m_physique_num==0) 
		return true;

	if(pNode->m_bConnectPhysique)
		return true;

	for(int i=0;i<pNode->m_physique_num;i++) {

		p_num = pNode->m_physique[i].m_num;

		if(p_num > MAX_PHYSIQUE_KEY)	// 최대 4개 지원
			p_num = MAX_PHYSIQUE_KEY;

		for(j=0;j<p_num;j++) {

			ret = _FindMeshId(&pNode->m_physique[i].m_parent_name[j][0]);

			if(ret != -1) {
				pNode->m_physique[i].m_parent_id[j] = ret;
			}
			else {
				mlog("ReadElu() : physique %d %d %s not found !!\n",i,j,&pNode->m_physique[i].m_parent_name[j][0]);
				//	return false;
			}
		}
	}

	pNode->m_bConnectPhysique = true;

	if(RIsSupportVS())
		if( pNode->m_point_color_num == 0 )
			pNode->MakeVSVertexBuffer();

	return true;
}


//파츠교환때문에 미리연산하는것이 의미가 없음..
// 로딩시점에 자신의 머터리얼에 대해서 알파인가 조사..
void RMesh::CheckNodeAlphaMtrl() {

	RMeshNodeHashList_Iter it_obj =  m_list.begin();

	RMeshNode*	pMeshNode = NULL;
	RMtrlMgr*  pMtrlList = NULL;

	RMtrl* pMtrl,*pSMtrl;

	while (it_obj !=  m_list.end()) {
		pMeshNode = (*it_obj);

		if(pMeshNode->m_mtrl_id != -1) {
			if( m_mtrl_list_ex.GetNum()) {
				if(pMtrl = m_mtrl_list_ex.Get_s(pMeshNode->m_mtrl_id,-1)) {

					if( pMtrl->m_sub_mtrl_num) {

						for(int i=0;i<pMtrl->m_sub_mtrl_num;i++) {
							pSMtrl = m_mtrl_list_ex.Get_s(pMeshNode->m_mtrl_id,i);

							if( pSMtrl->m_bAlphaMap || pSMtrl->m_bDiffuseMap ){
								pMeshNode->m_isAlphaMtrl = true;
								break;
							}
						}
					}
					else {
						if( pMtrl->m_bAlphaMap || pMtrl->m_bDiffuseMap )
							pMeshNode->m_isAlphaMtrl = true;
					}
				}
			}
		}

		it_obj++;
		continue;
	}
}

void RMesh::ClearVoidMtrl()
{
	RMeshNode*	pMeshNode = NULL;
	RMtrl*		pMtrl = NULL;
	RMtrl*		pSMtrl = NULL;

	int mtrl_size = m_mtrl_list_ex.size();

	if(!mtrl_size)
		return;

	m_mtrl_list_ex.ClearUsedCheck();

	RMeshNodeHashList_Iter it_obj =  m_list.begin();

	while (it_obj !=  m_list.end()) {

		pMeshNode = (*it_obj);

		if(pMeshNode) {

			pMtrl = m_mtrl_list_ex.Get_s(pMeshNode->m_mtrl_id,-1);

			if(pMtrl) {

				pMtrl->m_bUse = true;

				if( pMtrl->m_sub_mtrl_num ) {//sub mtrl

					for(int i=0;i<pMtrl->m_sub_mtrl_num;i++) {

						pSMtrl = m_mtrl_list_ex.Get_s(pMeshNode->m_mtrl_id,i);

						if(pSMtrl) {
							pSMtrl->m_bUse = true;
						}
					}
				}
			}
		}
		it_obj++;
	}

	m_mtrl_list_ex.ClearUsedMtrl();

	mtrl_size = m_mtrl_list_ex.size();
}


_NAMESPACE_REALSPACE2_END
