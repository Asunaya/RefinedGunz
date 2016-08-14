#include "BasicInfoHistory.h"

template <typename GetItemDescT>
bool BasicInfoHistoryManager::GetPositions(v3* OutHead, v3* OutFoot, v3* OutDir, double Time,
	GetItemDescT& GetItemDesc, MMatchSex Sex, bool IsDead) const
{
	auto Return = [&](const v3& Head, const v3& Foot, const v3& Dir)
	{
		if (OutHead)
			*OutHead = Head;
		if (OutFoot)
			*OutFoot = Foot;
		if (OutDir)
			*OutDir = Dir;
	};

	auto GetHead = [&](const v3& Pos, const v3& Dir, auto pre_it, auto LowerFrameTime, auto UpperFrameTime)
	{
		auto ItemDesc = GetItemDesc(pre_it->SelectedSlot);
		auto MotionType = eq_weapon_etc;
		if (ItemDesc)
			MotionType = WeaponTypeToMotionType(ItemDesc->m_nWeaponType);

		auto LowerAni = GetAnimationMgr(Sex)->GetAnimation(g_AnimationInfoTableLower[pre_it->lowerstate].Name, MotionType);
		RAnimation* UpperAni = nullptr;
		bool HasUpperAni = pre_it->upperstate != ZC_STATE_UPPER_NONE;
		if (HasUpperAni)
			UpperAni = GetAnimationMgr(Sex)->GetAnimation(g_AnimationInfoTableUpper[pre_it->upperstate].Name, MotionType);
		if (!LowerAni)
			return Pos + v3(0, 0, 180);

		int LowerFrame = GetFrame(*LowerAni, pre_it->lowerstate, ItemDesc, static_cast<float>(LowerFrameTime));
		int UpperFrame = 0;
		if (HasUpperAni)
			UpperFrame = GetFrame(*UpperAni, ZC_STATE_LOWER(0), nullptr, static_cast<float>(UpperFrameTime));

		return GetAbsHead(Pos, Dir, Sex,
			pre_it->lowerstate, pre_it->upperstate,
			LowerFrame, UpperFrame,
			MotionType, IsDead);
	};

	if (BasicInfoList.empty())
	{
		Return({ 0, 0, 180 }, { 0, 0, 0 }, { 1, 0, 0 });
		return true;
	}

	if (BasicInfoList.size() == 1)
	{
		auto it = BasicInfoList.begin();
		Return(GetHead(it->position, it->direction, it, it->LowerFrameTime, it->UpperFrameTime), it->position, it->direction);
		return true;
	}

	auto pre_it = BasicInfoList.begin();
	auto post_it = BasicInfoList.begin();

	while (pre_it != BasicInfoList.end() && Time < pre_it->RecvTime)
	{
		post_it = pre_it;
		pre_it++;
	}

	if (pre_it == BasicInfoList.end())
		pre_it = post_it;

	v3 AbsPos;
	v3 Dir;
	double LowerFrameTime;
	double UpperFrameTime;

	if (pre_it != post_it)
	{
		auto t = double(Time - pre_it->RecvTime) / double(post_it->RecvTime - pre_it->RecvTime);
		AbsPos = Lerp(pre_it->position, post_it->position, static_cast<float>(t));
		Dir = Slerp(pre_it->direction, post_it->direction, static_cast<float>(t));
		LowerFrameTime = Lerp(pre_it->LowerFrameTime, post_it->LowerFrameTime, static_cast<float>(t));
		UpperFrameTime = Lerp(pre_it->UpperFrameTime, post_it->UpperFrameTime, static_cast<float>(t));
	}
	else
	{
		AbsPos = pre_it->position;
		Dir = pre_it->direction;
		LowerFrameTime = pre_it->LowerFrameTime + (Time - pre_it->RecvTime);
		UpperFrameTime = pre_it->UpperFrameTime + (Time - pre_it->RecvTime);
	}

	Return(GetHead(AbsPos, Dir, pre_it, LowerFrameTime, UpperFrameTime), AbsPos, Dir);

	return true;
}