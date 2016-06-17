#pragma once

#include "AnimationStuff.h"
#include "MDebug.h"
#define _WINSOCK2API_
#undef pi
#include "MMatchItem.h"
#include <array>

static RAnimationMgr* AniMgrs[2];

RAnimationMgr * GetAnimationMgr(MMatchSex Sex)
{
	return AniMgrs[Sex];
}

void SetAnimationMgr(MMatchSex Sex, RAnimationMgr* AniMgr)
{
	AniMgrs[Sex] = AniMgr;
}

struct NodeInfo
{
	const char* Name = "";
	RMeshPartsPosInfoType PosParts;
	CutParts cut_parts;
	const NodeInfo* Parent;
	const NodeInfo* Child;
};

static const std::array<NodeInfo, eq_parts_pos_info_end> Nodes = []()
{
	std::array<NodeInfo, eq_parts_pos_info_end> infos;

	auto i = eq_parts_pos_info_Root;
	auto last_i = i;
	infos[i].Name = "Bip01";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_lower_body;
	infos[i].Parent = nullptr;

	last_i = i;
	i = eq_parts_pos_info_Pelvis;
	infos[i].Name = "Bip01 Pelvis";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_lower_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_Spine;
	infos[i].Name = "Bip01 Spine";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_lower_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_Spine1;
	infos[i].Name = "Bip01 Spine1";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_Spine2;
	infos[i].Name = "Bip01 Spine2";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_Neck;
	infos[i].Name = "Bip01 Neck";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_Head;
	infos[i].Name = "Bip01 Head";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_HeadNub;
	infos[i].Name = "Bip01 HeadNub";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = eq_parts_pos_info_Spine;
	i = eq_parts_pos_info_RThigh;
	infos[i].Name = "Bip01 R Thigh";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_lower_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_RCalf;
	infos[i].Name = "Bip01 R Calf";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_lower_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_RFoot;
	infos[i].Name = "Bip01 R Foot";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_lower_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_RToe0;
	infos[i].Name = "Bip01 R Toe0";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_lower_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_RToe0Nub;
	infos[i].Name = "Bip01 R Toe0Nub";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_lower_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = eq_parts_pos_info_Spine;
	i = eq_parts_pos_info_LThigh;
	infos[i].Name = "Bip01 L Thigh";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_lower_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_LCalf;
	infos[i].Name = "Bip01 L Calf";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_lower_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_LFoot;
	infos[i].Name = "Bip01 L Foot";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_lower_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_LToe0;
	infos[i].Name = "Bip01 L Toe0";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_lower_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_LToe0Nub;
	infos[i].Name = "Bip01 L Toe0Nub";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_lower_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = eq_parts_pos_info_Neck;
	i = eq_parts_pos_info_RClavicle;
	infos[i].Name = "Bip01 R Clavicle";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_RUpperArm;
	infos[i].Name = "Bip01 R UpperArm";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_RForeArm;
	infos[i].Name = "Bip01 R ForeArm";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_RHand;
	infos[i].Name = "Bip01 R Hand";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_RFinger0;
	infos[i].Name = "Bip01 R Finger0";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_RFingerNub;
	infos[i].Name = "Bip01 R Finger0Nub";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = eq_parts_pos_info_Neck;
	i = eq_parts_pos_info_LClavicle;
	infos[i].Name = "Bip01 L Clavicle";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_LUpperArm;
	infos[i].Name = "Bip01 L UpperArm";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_LForeArm;
	infos[i].Name = "Bip01 L ForeArm";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_LHand;
	infos[i].Name = "Bip01 L Hand";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_LFinger0;
	infos[i].Name = "Bip01 L Finger0";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	last_i = i;
	i = eq_parts_pos_info_LFingerNub;
	infos[i].Name = "Bip01 L Finger0Nub";
	infos[i].PosParts = i;
	infos[i].cut_parts = cut_parts_upper_body;
	infos[i].Parent = &Nodes[last_i];

	return infos;
}();

static void GetAniMat(matrix& mat, RAnimationNode& node, const matrix* parent_base_inv, int frame);
static void GetRotAniMat(matrix& mat, RAnimationNode& node, const matrix* parent_base_inv, int frame);
static void GetPosAniMat(matrix& mat, RAnimationNode& node, const matrix* parent_base_inv, int frame);
static void RotateSpine(matrix& mat, RMeshPartsPosInfoType parts, float y, float tremble);

template <size_t size>
static matrix GetNodeHierarchyMatrix(RAnimation* LowerAni, RAnimation* UpperAni,
	const RMeshPartsPosInfoType(&Hierarchy)[size],
	int Frame, float y, float tremble)
{
	DMLog("Lower: %p, %s; upper: %p, %s\n", LowerAni, LowerAni ? LowerAni->GetName() : "(null)",
		UpperAni, UpperAni ? UpperAni->GetName() : "(null)");
	matrix last_mat;
	matrix last_mat_inv;
	matrix mat;
	bool parent = false;
	rmatrix* last_mat_inv_ptr = nullptr;
	for (size_t i = 0; i < ArraySize(Hierarchy); i++)
	{
		auto& nodeinfo = Nodes[Hierarchy[i]];

		RAnimation* Ani = nullptr;
		if (UpperAni)
			Ani = nodeinfo.cut_parts == cut_parts_lower_body ? LowerAni : UpperAni;
		else
			Ani = LowerAni;

		auto& cur = *Ani->m_pAniData->GetNode(nodeinfo.Name);

		D3DXMatrixIdentity(&mat);

		if (nodeinfo.PosParts == eq_parts_pos_info_Spine1 && UpperAni)
		{
			GetUpperSpine1(mat, UpperAni, Frame, y, tremble);
		}
		else
		{
			GetAniMat(mat, cur, last_mat_inv_ptr, Frame);
			RotateSpine(mat, Hierarchy[i], y, tremble);
		}

		if (parent)
			mat *= last_mat;

		parent = true;
		last_mat = mat;
		RMatInv(last_mat_inv, cur.m_mat_base);
		last_mat_inv_ptr = &last_mat_inv;
	}

	return mat;
}

bool GetNodeMatrix(matrix& mat, const char* Name, const matrix* parent_base_inv,
	RAnimation* Ani, int Frame, float y, float tremble)
{
	const NodeInfo* nodeinfo = nullptr;
	for (auto& node : Nodes)
	{
		if (!strcmp(node.Name, Name))
		{
			nodeinfo = &node;
			break;
		}
	}
	
	//DMLog("%s: %p, %p\n", Name, nodeinfo, Ani);
	
	if (!nodeinfo)
		return false;

	if (!Ani)
		return false;

	auto& cur = *Ani->m_pAniData->GetNode(nodeinfo->Name);
	D3DXMatrixIdentity(&mat);
	GetAniMat(mat, cur, parent_base_inv, Frame);
	RotateSpine(mat, nodeinfo->PosParts, y, tremble);
	
	return true;
}

bool GetUpperSpine1(matrix& mat, RAnimation* Ani, int Frame, float y, float tremble)
{
	auto& spine1 = *Ani->m_pAniData->GetNode("Bip01 Spine1");
	auto& spine = *Ani->m_pAniData->GetNode("Bip01 Spine");
	matrix spine_base_inv = spine.m_mat_base;
	RMatInv(spine_base_inv, spine_base_inv);
	GetAniMat(mat, spine1, &spine_base_inv, Frame);
	RotateSpine(mat, eq_parts_pos_info_Spine1, y, tremble);
	return true;
}

v3 GetHeadPosition(RAnimation* LowerAni, RAnimation* UpperAni, float y, int Frame, float tremble)
{
	static const RMeshPartsPosInfoType Hierarchy[] = { eq_parts_pos_info_Root, eq_parts_pos_info_Pelvis,
		eq_parts_pos_info_Spine, eq_parts_pos_info_Spine1, eq_parts_pos_info_Spine2,
		eq_parts_pos_info_Neck, eq_parts_pos_info_Head };

	matrix mat = GetNodeHierarchyMatrix(LowerAni, UpperAni, Hierarchy, Frame, y, tremble);

	return GetTransPos(mat);
}

v3 GetFootPosition(RAnimation* LowerAni, int Frame)
{
	static const RMeshPartsPosInfoType HierarchyR[] = { eq_parts_pos_info_Root, eq_parts_pos_info_Pelvis,
		eq_parts_pos_info_Spine, eq_parts_pos_info_RThigh, eq_parts_pos_info_RCalf, eq_parts_pos_info_RFoot };
	static const RMeshPartsPosInfoType HierarchyL[] = { eq_parts_pos_info_Root, eq_parts_pos_info_Pelvis,
		eq_parts_pos_info_Spine, eq_parts_pos_info_LThigh, eq_parts_pos_info_LCalf, eq_parts_pos_info_LFoot };

	matrix matR = GetNodeHierarchyMatrix(LowerAni, nullptr, HierarchyR, Frame, 0, 0);
	matrix matL = GetNodeHierarchyMatrix(LowerAni, nullptr, HierarchyL, Frame, 0, 0);

	v3 ret = GetTransPos(matR) + GetTransPos(matL);
	ret /= 2;
	ret.y -= 12;
	return ret;
}

static void GetAniMat(matrix& mat, RAnimationNode& node, const matrix* parent_base_inv, int frame)
{
	if (node.m_mat_cnt)
	{
		mat = node.GetTMValue(frame);
		return;
	}

	bool HasPosValue = node.m_pos_cnt != 0;
	bool HasRotValue = node.m_rot_cnt != 0;

	if (!HasPosValue && !HasRotValue)
	{
		mat = node.m_mat_base;

		if (parent_base_inv)
			mat *= *parent_base_inv;

		return;
	}

	GetRotAniMat(mat, node, parent_base_inv, frame);
	GetPosAniMat(mat, node, parent_base_inv, frame);
}

static void GetRotAniMat(matrix& mat, RAnimationNode& node, const matrix* parent_base_inv, int frame)
{
	D3DXMATRIX buffer, Inv;

	bool bAni = false;

	if (node.m_rot_cnt)
		bAni = true;

	if (bAni) {
		D3DXQUATERNION out = node.GetRotValue(frame);

		D3DXMatrixRotationQuaternion(&mat, &out);
	}
	else {

		D3DXMatrixIdentity(&buffer);

		buffer = node.m_mat_base;

		if (parent_base_inv)
			buffer *= *parent_base_inv;

		buffer._41 = buffer._42 = buffer._43 = 0;
		mat *= buffer;
	}
}

static void GetPosAniMat(matrix& mat, RAnimationNode& node, const matrix* parent_base_inv, int frame)
{
	D3DXMATRIX buffer, Inv;

	bool bAni = false;

	if (node.m_pos_cnt)
	{
		bAni = true;
	}

	if (bAni) {
		auto pos = node.GetPosValue(frame);

		for (int i = 0; i < 3; i++)
			mat(3, i) = pos[i];
	}
	else {

		D3DXMatrixIdentity(&buffer);

		buffer = node.m_mat_base;

		if (parent_base_inv)
			buffer *= *parent_base_inv;

		for (int i = 0; i < 3; i++)
			mat(3, i) = buffer(3, i);
	}
}

static void RotateSpine(matrix& mat, RMeshPartsPosInfoType parts, float y, float tremble)
{
	float ratio = 0;

	switch (parts)
	{
	case eq_parts_pos_info_Head:
		ratio = 0.3;
		break;
	case eq_parts_pos_info_Spine1:
		ratio = 0.6;
		break;
	case eq_parts_pos_info_Spine2:
		ratio = 0.5;
		break;
	default:
		return;
	};

	float y_clamped = y;

	constexpr auto MAX_YA_FRONT = 50;
	constexpr auto MAX_YA_BACK = -70.f;

	if (y_clamped > MAX_YA_FRONT)
		y_clamped = MAX_YA_FRONT;

	if (y_clamped < MAX_YA_BACK)
		y_clamped = MAX_YA_BACK;

	auto my = RGetRotY((y_clamped + tremble) * ratio);

	mat *= my;
}

static float GetSpeed(ZC_STATE_LOWER LowerState, int MaxFrame, MMatchItemDesc* ItemDesc)
{
	float ret = 4.8;

	[&]()
	{
		if (!ItemDesc)
			return;

		if (ItemDesc->m_nType != MMIT_MELEE)
			return;

		switch (LowerState)
		{
		case ZC_STATE_LOWER_ATTACK1:
		case ZC_STATE_LOWER_ATTACK1_RET:
		case ZC_STATE_LOWER_ATTACK2:
		case ZC_STATE_LOWER_ATTACK2_RET:
		case ZC_STATE_LOWER_ATTACK3:
		case ZC_STATE_LOWER_ATTACK3_RET:
		case ZC_STATE_LOWER_ATTACK4:
		case ZC_STATE_LOWER_ATTACK4_RET:
		case ZC_STATE_LOWER_ATTACK5:
		case ZC_STATE_LOWER_JUMPATTACK:
			break;
		case ZC_STATE_SLASH:
			ret *= 0.9;
			return;
		default:
			return;
		};

		int Delay = GetSelectWeaponDelay(ItemDesc);

		int Time = MaxFrame / 4.8;

		int AttackSpeed = Time + Delay;
		if (AttackSpeed < 1)
			AttackSpeed = 1;

		ret = float(Time) / AttackSpeed * 4.8;
	}();

	return ret;
}

int GetSelectWeaponDelay(MMatchItemDesc* pSelectItemDesc)
{
	if (!pSelectItemDesc) return 0;

	int nReturnDelay = pSelectItemDesc->m_nDelay;

	if (pSelectItemDesc->m_nType != MMIT_MELEE)
		return 0;

	switch (pSelectItemDesc->m_nWeaponType)
	{
	case MWT_DAGGER:
		nReturnDelay -= 266;
		break;

	case MWT_DUAL_DAGGER:
		nReturnDelay -= 299;
		break;

	case MWT_KATANA:
		nReturnDelay -= 299;
		break;

	case MWT_DOUBLE_KATANA:
		nReturnDelay -= 299;
		break;

	case MWT_GREAT_SWORD:
		nReturnDelay -= 399;
		break;

	default:
		_ASSERT(0);
		break;
	}

	return nReturnDelay;
}

int GetFrame(RAnimation& Ani, ZC_STATE_LOWER LowerState, MMatchItemDesc* ItemDesc, float Time)
{
	if (Time < 0)
		Time = 0;

	int Frame = Time * 1000 * GetSpeed(LowerState, Ani.GetMaxFrame(), ItemDesc);

	if (Ani.GetAnimationLoopType() == RAniLoopType_Loop)
		Frame %= Ani.GetMaxFrame();
	else if (Frame >= Ani.GetMaxFrame())
		Frame = Ani.GetMaxFrame() - 1;

	/*DMLog("Time: %f, %f, speed: %f, frame: %d\n", Time, Time * 1000,
		GetSpeed(LowerState, Ani.GetMaxFrame(), ItemDesc), Frame);*/

	return Frame;
}