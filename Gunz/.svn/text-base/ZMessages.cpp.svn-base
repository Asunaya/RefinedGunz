#include "stdafx.h"

#include "ZMessages.h"
#include "MXml.h"
#include "ZFilePath.h"

#include "ZApplication.h"


const char* ZGetSexStr(MMatchSex nSex, bool bShort)
{
	if (bShort)
	{
		switch (nSex)
		{
		case MMS_MALE:		
			return ZMsg(MSG_WORD_MALE);

		case MMS_FEMALE:	
			return ZMsg(MSG_WORD_FEMALE);
		}
	}
	else
	{
		switch (nSex)
		{
		case MMS_MALE:		
			return ZMsg(MSG_WORD_MALE_SHORT);

		case MMS_FEMALE:	
			return ZMsg(MSG_WORD_FEMALE_SHORT);
		}
	}

	_ASSERT(0);
	return "";
}

void ZGetTimeStrFromSec(char* poutStr, unsigned long int nSec)
{
	int d, h, m, s;

	d = (nSec / (60*60*24));
	nSec = nSec % (60*60*24);

	h = (nSec / (60*60));
	nSec = nSec % (60*60);

	m = (nSec / (60));
	nSec = nSec % (60);
	
	s = nSec;

	char sztemp[64];

	poutStr[0] = 0;
	if (d != 0)
	{
		sprintf(sztemp, "%d%s ", d, ZMsg( MSG_CHARINFO_DAY));
		strcat(poutStr, sztemp);
	}
	if (h != 0)
	{
		sprintf(sztemp, "%d%s ", h, ZMsg( MSG_CHARINFO_HOUR));
		strcat(poutStr, sztemp);
	}
	if (m != 0)
	{
		sprintf(sztemp, "%d%s ", m, ZMsg( MSG_CHARINFO_MINUTE));
		strcat(poutStr, sztemp);
	}
	sprintf(sztemp, "%d%s", s, ZMsg( MSG_CHARINFO_SECOND));
	strcat(poutStr, sztemp);
}

