#ifndef _MMATCHMAP_H
#define _MMATCHMAP_H


#define MAPNAME_LENGTH		32



enum MMATCH_MAP 
{
	MMATCH_MAP_MANSION			= 0,
	MMATCH_MAP_PRISON			= 1,
	MMATCH_MAP_STATION			= 2,
	MMATCH_MAP_PRISON_II		= 3,
	MMATCH_MAP_BATTLE_ARENA		= 4,
	MMATCH_MAP_TOWN				= 5,
	MMATCH_MAP_DUNGEON			= 6,
	MMATCH_MAP_RUIN				= 7,
	MMATCH_MAP_ISLAND			= 8,
	MMATCH_MAP_GARDEN			= 9,
	MMATCH_MAP_CASTLE			= 10,
	MMATCH_MAP_FACTORY			= 11,
	MMATCH_MAP_PORT				= 12,
	MMATCH_MAP_LOST_SHRINE		= 13,
	MMATCH_MAP_STAIRWAY			= 14,
	MMATCH_MAP_SNOWTOWN			= 15,
	MMATCH_MAP_HALL				= 16,
	MMATCH_MAP_CATACOMB			= 17,
	MMATCH_MAP_JAIL				= 18,
	MMATCH_MAP_SHOWERROOM		= 19,
	// 새맵 추가는 이 밑에 하세요.


	// 새맵 추가는 이 위에 하세요.
//	MMATCH_MAP_EVENT,					// 이벤트 전용맵

	MMATCH_MAP_MAX
};

#define MMATCH_MAP_COUNT	MMATCH_MAP_MAX			// 전체 맵 갯수


const struct MMatchMapDesc
{
	int			nMapID;							// map id
	char		szMapName[MAPNAME_LENGTH];		// 맵 이름
	char		szMapImageName[MAPNAME_LENGTH];	// 맵 이미지 이름
	char		szBannerName[MAPNAME_LENGTH];	// 베너 이름
	float		fExpRatio;						// 경험치 습득률
	int			nMaxPlayers;					// 최대 인원
	bool		bOnlyDuelMap;					// 듀얼맵 전용

} g_MapDesc[MMATCH_MAP_COUNT] = 
{ 
	{MMATCH_MAP_MANSION,		"Mansion",		"map_Mansion.bmp",		"banner_Mansion.tga",		1.0f,	16,		false}, 
	{MMATCH_MAP_PRISON,			"Prison",		"map_Prison.bmp",		"banner_Prison.tga",		1.0f,	16,		false}, 
	{MMATCH_MAP_STATION,		"Station",		"map_Station.bmp",		"banner_Station.tga",		1.0f,	16,		false}, 
	{MMATCH_MAP_PRISON_II,		"Prison II",	"map_Prison II.bmp",	"banner_Prison II.tga",		1.0f,	16,		false},
	{MMATCH_MAP_BATTLE_ARENA,	"Battle Arena",	"map_Battle Arena.bmp",	"banner_Battle Arena.tga",	1.0f,	16,		false},
	{MMATCH_MAP_TOWN,			"Town",			"map_Town.bmp",			"banner_Town.tga",			1.0f,	16,		false},
	{MMATCH_MAP_DUNGEON,		"Dungeon",		"map_Dungeon.bmp",		"banner_Dungeon.tga",		1.0f,	16,		false},
	{MMATCH_MAP_RUIN,			"Ruin",			"map_Ruin.bmp",			"banner_Ruin.tga",			1.0f,	16,		false},
	{MMATCH_MAP_ISLAND,			"Island",		"map_island.bmp",		"banner_island.tga",		1.0f,	16,		false},
	{MMATCH_MAP_GARDEN,			"Garden",		"map_Garden.bmp",		"banner_Garden.tga",		1.0f,	16,		false},
	{MMATCH_MAP_CASTLE,			"Castle",		"map_Castle.bmp",		"banner_Castle.tga",		1.0f,	16,		false},
	{MMATCH_MAP_FACTORY,		"Factory",		"map_Factory.bmp",		"banner_Factory.tga",		0.8f,	8,		false},
	{MMATCH_MAP_PORT,			"Port",			"map_Port.bmp",			"banner_port.tga",			1.0f,	16,		false},
	{MMATCH_MAP_LOST_SHRINE,	"Lost Shrine",	"map_Lost Shrine.bmp",	"banner_Lost Shrine.tga",	1.0f,	16,		false},
	{MMATCH_MAP_STAIRWAY,		"Stairway",		"map_Stairway.bmp",		"banner_Stairway.tga",		1.0f,	16,		false},
	{MMATCH_MAP_SNOWTOWN,		"Snow_Town",	"map_Snow_Town.bmp",	"banner_Snow_Town.tga",		1.1f,	16,		false},
	{MMATCH_MAP_HALL,			"Hall",			"map_Mansion.bmp",		"banner_Mansion.tga",		1.0f,	16,		true},
	{MMATCH_MAP_CATACOMB,		"Catacomb",		"map_Dungeon.bmp",		"banner_Dungeon.tga",		1.0f,	16,		true},
	{MMATCH_MAP_JAIL,			"Jail",			"map_Prison.bmp",		"banner_Prison.tga",		1.0f,	16,		true},
	{MMATCH_MAP_SHOWERROOM,		"Shower Room",	"map_Prison II.bmp",	"banner_Prison II.tga",		1.0f,	16,		true},
};


inline bool MIsCorrectMap(const int nMapID)
{
	if ((nMapID < 0) || (nMapID >= MMATCH_MAP_MAX)) return false;
	return true;
}

inline const char* MGetMapName(const int nMapID)
{
	if (MIsCorrectMap(nMapID))
		return g_MapDesc[nMapID].szMapName;
	else
		return 0;
}

inline const char* MGetMapImageName(const char* szMapName)
{
	for ( int i = 0;  i < MMATCH_MAP_COUNT;  i++)
	{
		if ( stricmp( szMapName, g_MapDesc[ i].szMapName) == 0)
			return g_MapDesc[ i].szMapImageName;
	}

	return 0;
}

inline const char* MGetBannerName(const char* szMapName)
{
	for ( int i = 0;  i < MMATCH_MAP_COUNT;  i++)
	{
		if ( stricmp( szMapName, g_MapDesc[ i].szMapName) == 0)
			return g_MapDesc[ i].szBannerName;
	}

	return 0;
}

inline bool MIsMapOnlyDuel( const int nMapID)
{
	if ( MIsCorrectMap(nMapID))
		return g_MapDesc[ nMapID].bOnlyDuelMap;
	else
		return 0;
}

#endif