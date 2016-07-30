#include "stdafx.h"
#include "BasicInfoHistory.h"

void BasicInfoHistoryManager::AddBasicInfo(const BasicInfoItem& bii)
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
			local.LowerFrameTime = prev_it->LowerFrameTime + (local.RecvTime - prev_it->RecvTime);
		}

		if (prev_it->upperstate != local.upperstate)
		{
			local.UpperFrameTime = local.upperstate == ZC_STATE_UPPER_NONE ? -1 : 0;
		}
		else
		{
			local.UpperFrameTime = local.upperstate == ZC_STATE_UPPER_NONE ? -1
				: prev_it->UpperFrameTime + (local.RecvTime - prev_it->RecvTime);
		}
	}

	BasicInfoList.push_front(local);

	while (BasicInfoList.size() > 1000)
	{
		BasicInfoList.pop_back();
	}
}