#ifndef _CUSTOM_DEFINE
#define _CUSTOM_DEFINE


#include "MMatchTransDataType.h"

#define MPACKET_HEADER_SIZE				(sizeof(MPacketHeader))
#define MTD_SERVER_STATUS_INFO_SIZE		(sizeof(MTD_ServerStatusInfo))

/* MMatchGlobal.h∑Œ ¿Ãµø.
enum SERVER_TYPE
{
	ST_DEBUG = 1,
	ST_NORMAL,
	ST_CLAN,
	ST_QUEST,
	ST_EVENT,
	ST_TEST,
};
*/

#define VALID_COUNTRY_CODE_FILE ".\\ValidCountryCode.txt"

#endif