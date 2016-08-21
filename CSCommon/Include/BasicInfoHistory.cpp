#include "stdafx.h"
#include "BasicInfoHistory.h"

void BasicInfoHistoryManager::AddBasicInfo(BasicInfoItem bii)
{
	if (BasicInfoList.empty())
	{
		bii.LowerFrameTime = 0;
		bii.UpperFrameTime = bii.upperstate == ZC_STATE_UPPER_NONE ? -1 : 0;
		if (bii.lowerstate == -1)
		{
			bii.lowerstate = ZC_STATE_LOWER_IDLE1;
			bii.upperstate = ZC_STATE_UPPER_NONE;
		}

		if (bii.SelectedSlot == -1)
			bii.SelectedSlot = MMCIP_PRIMARY;
	}
	else
	{
		auto prev_it = BasicInfoList.begin();

		if (bii.lowerstate == -1)
		{
			bii.lowerstate = prev_it->lowerstate;
			bii.upperstate = prev_it->upperstate;
		}

		if (bii.SelectedSlot == -1)
			bii.SelectedSlot = prev_it->SelectedSlot;

		if (prev_it->lowerstate != bii.lowerstate)
		{
			bii.LowerFrameTime = 0;
		}
		else
		{
			bii.LowerFrameTime = prev_it->LowerFrameTime + (bii.RecvTime - prev_it->RecvTime);
		}

		if (prev_it->upperstate != bii.upperstate)
		{
			bii.UpperFrameTime = bii.upperstate == ZC_STATE_UPPER_NONE ? -1 : 0;
		}
		else
		{
			bii.UpperFrameTime = bii.upperstate == ZC_STATE_UPPER_NONE ? -1
				: prev_it->UpperFrameTime + (bii.RecvTime - prev_it->RecvTime);
		}
	}

	BasicInfoList.push_front(bii);

	while (BasicInfoList.size() > 1000)
	{
		BasicInfoList.pop_back();
	}
}