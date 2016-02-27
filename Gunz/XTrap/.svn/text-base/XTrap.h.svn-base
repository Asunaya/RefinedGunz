
/******************************************************************************
// XTrap.h Version 0xD5000001->0xD5000005
// By WiseLogic
//****************************************************************************/

#ifndef __XTRAPMODULE_H
#define __XTRAPMODULE_H

#define		CREATEKF_OUT_SIZE			544
#define		CREATEKF_IN_SIZE			264

//////////////////////////////////////////////////////////////////////////////
// XTrap Interface External Variable
///////////////////////////////////////////////////////////////////////////////

extern BOOL g_bApiMal;			/* XTRAP_API_MDL	   : ACTIVE_DETECT_HACK : ACTIVE_FLAG_NONE */
extern BOOL g_bMemoryMdl;		/* XTRAP_MEMORY_MDL    : ACTIVE_DETECT_HACK : ACTIVE_FLAG_NONE */
extern BOOL g_bAutoMousMdl;		/* XTRAP_AUTOMOUS_MDL  : ACTIVE_DETECT_HACK : ACTIVE_FLAG_NONE */
extern BOOL g_bAutoKeybMdl;		/* XTRAP_AUTOKEYB_MDL  : ACTIVE_DETECT_HACK : ACTIVE_FLAG_NONE */
extern BOOL g_bMalMdl;			/* XTRAP_MAL_MDL       : ACTIVE_DETECT_HACK : ACTIVE_FLAG_NONE */
extern BOOL g_bSpeedMdl;		/* XTRAP_SPEED_MDL     : ACTIVE_DETECT_HACK : ACTIVE_FLAG_NONE */
extern BOOL g_bFileMdl;			/* XTRAP_FILE_MDL      : ACTIVE_DETECT_HACK : ACTIVE_FLAG_NONE */
extern BOOL g_bApiHookMdl;		/* XTRAP_APIHOOK_MDL   : ACTIVE_DETECT_HACK : ACTIVE_FLAG_NONE */
extern BOOL g_bDebugModMdl;		/* XTRAP_DEBUGMOD_MDL  : ACTIVE_DETECT_HACK : ACTIVE_FLAG_NONE */
extern BOOL g_bOsMdl;			/* XTRAP_OS_MDL        : ACTIVE_DETECT_HACK : ACTIVE_FLAG_NONE */
extern BOOL g_bPatchMdl;		/* XTRAP_PATCH_MDL	   : ACTIVE_DETECT_HACK : ACTIVE_FLAG_NONE */

extern BOOL g_bMemoryCrack;		/* XTRAP_MEMORY_CRACK  : ACTIVE_DETECT_HACK : ACTIVE_FLAG_NONE */
extern BOOL g_bFileCrack;		/* XTRAP_FILE_CRACK    : ACTIVE_DETECT_HACK : ACTIVE_FLAG_NONE */
extern BOOL g_bApiHookCrack;	/* XTRAP_APIHOOK_CRACK : ACTIVE_DETECT_HACK : ACTIVE_FLAG_NONE */
extern BOOL g_bApiCrack;		/* XTRAP_API_CRACK	   : ACTIVE_DETECT_HACK : ACTIVE_FLAG_NONE */

extern BOOL g_bStartXTrap;		/* XTrap cannot find the file/path specified       */
								/* XTrap program reguires a newer version of Games */

///////////////////////////////////////////////////////////////////////////////
// X-Trap Interface External Function
///////////////////////////////////////////////////////////////////////////////

/*required*/void SetXTrapStartInfo		(IN CHAR *pGameName, 
										 IN DWORD ApiVersion, 
									     IN DWORD VendorCode, 
										 IN DWORD KeyboardType, 
										 IN DWORD PatchType, 
										 IN DWORD ModuleType);
/*required*/void XTrapStart				();
/*required*/void XTrapKeepAlive			();

/*optional*/void SetXTrapMgrInfo		(IN CHAR *pMgrIp);
/*optional*/void SetXTrapPatchHttpUrl	(IN CHAR *pHttpUrl);
/*optional*/void SetOptGameInfo			(IN CHAR *pUserName,  
										 IN CHAR *pGameServerName, 
										 IN CHAR *pCharacterName, 
									     IN CHAR *pClassName, 
										 IN long UserLevel);

/*optional*/DWORD CreateKFEx			(IN  PUCHAR	pVerHash,	/* XTrapCC Version Verify Hash Value					*/
										 IN  PUCHAR	pInBuf,		/* Encyption[AddressTable(256byte) + RandomKey(8byte)]	*/
										 IN	 UCHAR	CMF_Cnt,	/* ServerSide CMF Count									*/
										 OUT PUCHAR	pOutBuf);	/* Generate Key											*/
/*Optional*/void SendLogToMgr			();

/*optional*/DWORD CreateKF				(IN char *VerHash, IN char *InBuf, OUT char *OutBuf);

///////////////////////////////////////////////////////////////////////////////
// X-Trap Patch Interface Function
///////////////////////////////////////////////////////////////////////////////

/*optional*/VOID XPatchStart			(IN DWORD VendorCode,	// VendorCode
										 IN LPSTR pPatchUrl,	// HTTP XPatch URL
										 IN LPSTR pGamePath		// Game Application Path (Default="", example=C:\\XGame)
										);

#endif
