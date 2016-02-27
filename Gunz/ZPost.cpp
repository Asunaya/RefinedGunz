#include "stdafx.h"

#include "ZPost.h"
#include "MBlobArray.h"
#include "MMatchTransDataType.h"
#include "MMatchGlobal.h"
#include "ZGame.h"
#include "ZMyCharacter.h"
#include "ZGameClient.h"
#include "ZApplication.h"
#include "ZConfiguration.h"


void ZPostUserOption()
{
	unsigned long nOptionFlags = 0;

	if (Z_ETC_REJECT_WHISPER)
		nOptionFlags |= MBITFLAG_USEROPTION_REJECT_WHISPER;
	if (Z_ETC_REJECT_INVITE)
		nOptionFlags |= MBITFLAG_USEROPTION_REJECT_INVITE;

	ZPOSTCMD1(MC_MATCH_USER_OPTION, MCmdParamUInt(nOptionFlags));
}
