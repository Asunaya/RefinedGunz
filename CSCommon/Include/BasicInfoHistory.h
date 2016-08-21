#pragma once

#include "stuff.h"
#include <deque>
#include "AnimationStuff.h"

class BasicInfoHistoryManager
{
public:
	void AddBasicInfo(BasicInfoItem bii);

	struct Info
	{
		v3 Head;
		v3 Origin;
		v3 Dir;
		v3 CameraDir;
	};

	template <typename GetItemDescT>
	bool GetInfo(Info& Out, double Time,
		GetItemDescT& GetItemDesc, MMatchSex Sex, bool IsDead) const;
	
	bool empty() const { return BasicInfoList.empty(); }
	auto& front() const { return BasicInfoList.front(); }
	void clear() { BasicInfoList.clear(); }

private:
	std::deque<BasicInfoItem> BasicInfoList;
};