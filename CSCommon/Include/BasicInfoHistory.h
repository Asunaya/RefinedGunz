#pragma once

#include "stuff.h"
#include <deque>
#include "AnimationStuff.h"

struct BasicInfoItem : public BasicInfo
{
	double SentTime;
	double RecvTime;

	BasicInfoItem() = default;
	BasicInfoItem(const BasicInfo& a, float b, u64 c)
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
		BasicInfoList.push_front(bii);

		while (BasicInfoList.size() > 1000)
		{
			BasicInfoList.pop_back();
		}
	}

	template <typename GetItemDescT>
	bool GetPositions(v3 & Head, v3 & Root, double Time,
		GetItemDescT& GetItemDesc, MMatchSex Sex);
	
	bool empty() const { return BasicInfoList.empty(); }
	auto& front() const { return BasicInfoList.front(); }

private:
	std::deque<BasicInfoItem> BasicInfoList;
};

template <typename GetItemDescT>
bool BasicInfoHistoryManager::GetPositions(v3 & Head, v3 & Root, double Time,
	GetItemDescT& GetItemDesc, MMatchSex Sex)
{
	if (BasicInfoList.size() < 2)
		return false;

	//DMLog("Time: %f, %f, %f\n", Time, BasicInfoList.begin()->RecvTime, (--BasicInfoList.end())->RecvTime);

	auto pre_it = BasicInfoList.begin();
	auto post_it = BasicInfoList.begin();

	while (pre_it != BasicInfoList.end() && Time < pre_it->RecvTime)
	{
		/*DMLog("Time = %f, pre_it = %f, post_it = %f\n",
			Time, pre_it->RecvTime, post_it->RecvTime);*/
		post_it = pre_it;
		pre_it++;
	}

	if (pre_it == BasicInfoList.end())
		pre_it = post_it;

	v3 AbsPos;
	v3 Dir;

	if (pre_it != post_it)
	{
		float t = double(Time - pre_it->RecvTime) / double(post_it->RecvTime - pre_it->RecvTime);
		AbsPos = Lerp(pre_it->position, post_it->position, t);
		Dir = Slerp(pre_it->direction, post_it->direction, t);

		DMLog("t: %f\n", t);

		/*MLog("t = %f; %f / %f; AbsPos = %f, %f, %f; Dir = %f, %f, %f",
		t, post_it->RecvTime - pre_it->RecvTime, Time - pre_it->RecvTime,
		AbsPos.x, AbsPos.y, AbsPos.z, Dir.x, Dir.y, Dir.z);*/
	}
	else
	{
		AbsPos = pre_it->position;
		Dir = pre_it->direction;
	}

	auto diff_ani_it = post_it;

	while (diff_ani_it != BasicInfoList.end() &&
		diff_ani_it->lowerstate == post_it->lowerstate)
	{
		diff_ani_it++;
	}

	if (diff_ani_it == BasicInfoList.end())
		diff_ani_it = BasicInfoList.end() - 1;

	auto Frame = u32(Time - diff_ani_it->RecvTime) * 1000;
	/*MLog("Ani %d; Frame = %d", pre_it->lowerstate, Frame);*/

	v3 xydir = Dir;
	xydir.z = 0;
	Normalize(xydir);

	matrix World;
	MakeWorldMatrix(&World, AbsPos, xydir, v3(0, 0, 1));

	float y = (Dir.z + 0.05) * 50;

	auto MotionType = eq_weapon_etc;

	[&]()
	{
		auto slot = pre_it->SelectedSlot;

		auto ItemDesc = GetItemDesc(slot);
		if (!ItemDesc)
			return;

		MotionType = WeaponTypeToMotionType(ItemDesc->m_nWeaponType);
	}();

	ZC_STATE_LOWER lowerstate = pre_it->lowerstate;

	auto Ani = GetAnimationMgr(Sex)->GetAnimation(g_AnimationInfoTableLower[lowerstate].Name, MotionType);

	Head = GetHeadPosition(*Ani, World, y, Frame);
	Root = AbsPos;

	//DMLog("Head: %f, %f, %f; foot: %f, %f, %f\n", Head.x, Head.y, Head.z, Root.x, Root.y, Root.z);

	return true;
}