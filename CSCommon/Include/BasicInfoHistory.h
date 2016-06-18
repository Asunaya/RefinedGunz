#pragma once

#include "stuff.h"
#include <deque>
#include "AnimationStuff.h"

struct BasicInfoItem : public BasicInfo
{
	double SentTime;
	double RecvTime;
	double LowerFrameTime;
	double UpperFrameTime;

	BasicInfoItem() = default;
	BasicInfoItem(const BasicInfo& a, float b, float c)
		: BasicInfo(a), SentTime(b), RecvTime(c)
	{ }
};

class BasicInfoHistoryManager
{
public:
	BasicInfoHistoryManager()
	{ }

	void AddBasicInfo(const BasicInfoItem& bii)
	{
		BasicInfoItem local = bii;

		if (BasicInfoList.empty())
		{
			local.LowerFrameTime = 0;
			local.UpperFrameTime = local.upperstate == ZC_STATE_UPPER_NONE ? -1 : 0;
		}
		else
		{
			auto prev_it = BasicInfoList.begin();

			if (prev_it->lowerstate != local.lowerstate)
			{
				local.LowerFrameTime = 0;
			}
			else
			{
				local.LowerFrameTime = prev_it->LowerFrameTime + (local.SentTime - prev_it->SentTime);
			}

			if (prev_it->upperstate != local.upperstate)
			{
				local.UpperFrameTime = local.upperstate == ZC_STATE_UPPER_NONE ? -1 : 0;
			}
			else
			{
				local.UpperFrameTime = local.upperstate == ZC_STATE_UPPER_NONE ? -1
					: prev_it->UpperFrameTime + (local.SentTime - prev_it->SentTime);
			}
		}

		BasicInfoList.push_front(local);

		while (BasicInfoList.size() > 1000)
		{
			BasicInfoList.pop_back();
		}
	}

	template <typename GetItemDescT>
	bool GetPositions(v3* OutHead, v3* OutFoot, v3* OutDir, double Time,
		GetItemDescT& GetItemDesc, MMatchSex Sex, bool IsDead);
	
	bool empty() const { return BasicInfoList.empty(); }
	auto& front() const { return BasicInfoList.front(); }
	void clear() { BasicInfoList.clear(); }

private:
	std::deque<BasicInfoItem> BasicInfoList;
};

template <typename GetItemDescT>
bool BasicInfoHistoryManager::GetPositions(v3* OutHead, v3* OutFoot, v3* OutDir, double Time,
	GetItemDescT& GetItemDesc, MMatchSex Sex, bool IsDead)
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

		int LowerFrame = GetFrame(*LowerAni, pre_it->lowerstate, ItemDesc, LowerFrameTime);
		int UpperFrame = 0;
		if (HasUpperAni)
			UpperFrame = GetFrame(*UpperAni, ZC_STATE_LOWER(0), nullptr, UpperFrameTime);

		float y = IsDead ? 0 : (Dir.z + 0.05) * 50;

		v3 Head = GetHeadPosition(LowerAni, UpperAni, LowerFrame, UpperFrame, y, 0);

		v3 xydir = Dir;
		xydir.z = 0;
		Normalize(xydir);

		v3 AdjPos = Pos;

		if (g_AnimationInfoTableLower[pre_it->lowerstate].bMove)
		{
			v3 Foot = GetFootPosition(LowerAni, LowerFrame);

			matrix WorldRot;
			MakeWorldMatrix(&WorldRot, { 0, 0, 0 }, xydir, { 0, 0, 1 });

			Foot *= WorldRot;

			AdjPos = Pos - Foot;
		}

		matrix World;
		MakeWorldMatrix(&World, AdjPos, xydir, v3(0, 0, 1));

		return Head * World;
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

	while (pre_it != BasicInfoList.end() && Time < pre_it->SentTime)
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
		auto t = double(Time - pre_it->SentTime) / double(post_it->SentTime - pre_it->SentTime);
		AbsPos = Lerp(pre_it->position, post_it->position, t);
		Dir = Slerp(pre_it->direction, post_it->direction, t);
		LowerFrameTime = Lerp(pre_it->LowerFrameTime, post_it->LowerFrameTime, t);
		UpperFrameTime = Lerp(pre_it->UpperFrameTime, post_it->UpperFrameTime, t);
	}
	else
	{
		AbsPos = pre_it->position;
		Dir = pre_it->direction;
		LowerFrameTime = pre_it->LowerFrameTime;
		UpperFrameTime = pre_it->UpperFrameTime;
	}

	Return(GetHead(AbsPos, Dir, pre_it, LowerFrameTime, UpperFrameTime), AbsPos, Dir);

	return true;
}