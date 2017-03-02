#include "stdafx.h"
#include "ZConfiguration.h"
#include "Mint.h"
#include "ZInterface.h"
#include "ZLocatorList.h"
#include "ZGameTypeList.h"

ZConfiguration	g_Configuration;
ZConfiguration* ZGetConfiguration()		{ return &g_Configuration; }

ZConfiguration::ZConfiguration()
{
	Init();

	strcpy_safe( m_szServerIP, "127.0.0.1");
	m_nServerPort = 6000;
	
	strcpy_safe( m_szBAReportAddr, "www.battlearena.com");
	strcpy_safe( m_szBAReportDir, "incoming");
	
	m_nServerCount = 0;

	m_pLocatorList = new ZLocatorList;
	m_pTLocatorList = new ZLocatorList;

	m_pGameTypeList = new ZGameTypeList;

	m_bIsComplete = false;
}

ZConfiguration::~ZConfiguration()
{
	Destroy();
	SAFE_DELETE(m_pLocatorList);
	SAFE_DELETE(m_pTLocatorList);
	SAFE_DELETE(m_pGameTypeList);
}

void ZConfiguration::Destroy()
{
}

unsigned long int GetVirtKey(const char *key)
{
	int n=atoi(key+1);
	if((key[0]=='f' || key[0]=='F') && n>=1 && n<=12)
		return VK_F1+n-1;

	if(key[0]>='a' && key[0]<='z')
		return 'A'+key[0]-'a';

	return key[0];
}

template<size_t size>
char *GetKeyName(unsigned long int nVirtKey, char(&out)[size]) {
	return GetKeyName(nVirtKey out, size);
}

char *GetKeyName(unsigned long int nVirtKey, char *out, int maxlen)
{
	if(nVirtKey>=VK_F1 && nVirtKey<=VK_F12)
		sprintf_safe(out, maxlen, "F%d",nVirtKey-VK_F1+1);
	else
		sprintf_safe(out, maxlen, "%d",nVirtKey);

	return out;
}

bool ZConfiguration::Load()
{
	bool retValue;

#if defined(_PUBLISH) && defined(ONLY_LOAD_MRS_FILES)
   		MZFile::SetReadMode( MZIPREADFLAG_ZIP | MZIPREADFLAG_MRS | MZIPREADFLAG_MRS2 | MZIPREADFLAG_FILE );
#endif

	if ( !LoadLocale(FILENAME_LOCALE) )
	{
		mlog( "Cannot open %s file.\n", FILENAME_LOCALE);
		return false;
	}

	if ( !LoadGameTypeCfg(FILENAME_GTCFG) )
	{
		mlog( "Cannot open %s file.\n", FILENAME_GTCFG);
		return false;
	}

	retValue = LoadConfig(FILENAME_CONFIG);

#if defined(_PUBLISH) && defined(ONLY_LOAD_MRS_FILES)
		MZFile::SetReadMode( MZIPREADFLAG_MRS2 );
#endif

	if (!LoadSystem(FILENAME_SYSTEM))
	{
		mlog( "Cannot open %s file.\n", FILENAME_SYSTEM);
		return false;
	}

	ConvertMacroStrings();

	if (!retValue)
		return false;

	return retValue;
}

bool ZConfiguration::LoadLocale(const char* szFileName)
{
	MXmlDocument	xmlLocale;
	MXmlElement		parentElement, serverElement, bindsElement;
	MXmlElement		childElement;

	char			*buffer;
	MZFile			mzFile;

	xmlLocale.Create();

	if( !mzFile.Open(szFileName, ZApplication::GetFileSystem()))
	{
		xmlLocale.Destroy();
		return false;
	} 

	buffer = new char[ mzFile.GetLength()+1];
	buffer[mzFile.GetLength()]=0;
	mzFile.Read( buffer, mzFile.GetLength());

	mlog( "Load XML from memory : %s", szFileName);

	if( !xmlLocale.LoadFromMemory(buffer) )
	{
		mlog( "- FAIL\n");

		xmlLocale.Destroy();
		return false;
	}
	delete[] buffer;
	mzFile.Close();
	mlog( "- SUCCESS\n");

	parentElement = xmlLocale.GetDocumentElement();
	int iCount = parentElement.GetChildNodeCount();

	if (!parentElement.IsEmpty())
	{
		if( parentElement.FindChildNode(ZTOK_LOCALE, &childElement) )
		{
			char szCountry[ 16 ]	= "";
			char szLanguage[ 16 ]	= "";
			int nMaxPlayers = 16;

			childElement.GetChildContents( szCountry, ZTOK_LOCALE_COUNTRY );
			childElement.GetChildContents( szLanguage, ZTOK_LOCALE_LANGUAGE );
			childElement.GetChildContents( &nMaxPlayers, ZTOK_LOCALE_MAXPLAYERS);

			if( (0 == szCountry) || (0 == szLanguage) )
			{
				mlog( "config.xml - Country or Language is invalid.\n" );
				return false;
			}

			m_Locale.strCountry		= szCountry;
			m_Locale.strLanguage	= szLanguage;
			m_Locale.nMaxPlayers	= nMaxPlayers;

			mlog( "Country : (%s), Language : (%s)\n", szCountry, szLanguage );
		}
	}
	xmlLocale.Destroy();

	return true;
}


bool ZConfiguration::LoadGameTypeCfg(const char* szFileName)
{
	MXmlDocument xmlIniData;
	xmlIniData.Create();

	char *buffer;
	MZFile mzFile;
	if( !mzFile.Open(szFileName, ZApplication::GetFileSystem())) 
	{
		xmlIniData.Destroy();
		return false;
	} 

	buffer = new char[ mzFile.GetLength()+1];
	buffer[mzFile.GetLength()]=0;
	mzFile.Read( buffer, mzFile.GetLength());

	mlog( "Load XML from memory : %s", szFileName);

	if( !xmlIniData.LoadFromMemory( buffer))
	{
		mlog( "- FAIL\n");

		xmlIniData.Destroy();
		delete []buffer;
		return false;
	}

	mlog("- SUCCESS\n");


	MXmlElement rootElement, chrElement, attrElement;

	char szTagName[ 256];

	rootElement = xmlIniData.GetDocumentElement();

	int iCount = rootElement.GetChildNodeCount();

	for ( int i = 0; i < iCount; i++)
	{
		chrElement = rootElement.GetChildNode( i);
		chrElement.GetTagName( szTagName);

		if (szTagName[0] == '#') continue;

		if ( !_stricmp( szTagName, ZTOK_GAME_TYPE)) 
		{
			int nID = 0;
			chrElement.GetAttribute( &nID, "id");

			m_pGameTypeList->ParseGameTypeList( nID, chrElement);
		}
	}

	xmlIniData.Destroy();
	delete []buffer;

	return true;
}

bool ZConfiguration::LoadSystem(const char* szFileName)
{
	char			*buffer;
	MZFile			mzFile;
	MXmlDocument	xmlConfig;
	xmlConfig.Create();

	if( !mzFile.Open( szFileName, ZApplication::GetFileSystem())) 
	{
		mlog("MFile::Open failed\n");
		xmlConfig.Destroy();
		return false;
	} 

	buffer = new char[ mzFile.GetLength()+1];
	buffer[mzFile.GetLength()]=0;
	mzFile.Read( buffer, mzFile.GetLength());

	mlog( "Load XML from memory : %s", FILENAME_SYSTEM );

	if( !xmlConfig.LoadFromMemory( buffer, GetLanguageID(m_Locale.strLanguage.c_str())) )
	{
		mlog( "- FAIL\n");

		xmlConfig.Destroy();
		return false;
	}
	delete[] buffer;
	mzFile.Close();
	mlog( "- SUCCESS\n");

	MXmlElement		parentElement = xmlConfig.GetDocumentElement();
	MXmlElement		serverElement, childElement;

	int iCount = parentElement.GetChildNodeCount();

	if (!parentElement.IsEmpty())
	{
		m_ServerList.clear();

		m_nServerCount = 0;
		while ( 1)
		{
			char szText[ 256];
			sprintf_safe( szText, "%s%d", ZTOK_SERVER, m_nServerCount);
			if (parentElement.FindChildNode( szText, &serverElement))
			{
				char szServerIP[ 32];
				char szName[ 32];
				int nServerPort;
				int nServerType;
				serverElement.GetChildContents( szServerIP,		ZTOK_IP);
				serverElement.GetChildContents( &nServerPort,	ZTOK_PORT);
				serverElement.GetChildContents( &nServerType,	ZTOK_TYPE);
				serverElement.GetChildContents( szName,			ZTOK_NAME);

                ZSERVERNODE ServerNode;
				strcpy_safe( ServerNode.szAddress, szServerIP);
				strcpy_safe( ServerNode.szName, szName);
				ServerNode.nPort = nServerPort;
				ServerNode.nType = nServerType;

				m_ServerList.insert( map<int,ZSERVERNODE>::value_type( m_nServerCount, ServerNode));

				m_nServerCount++;
			}
			else
				break;
		}

		if (parentElement.FindChildNode(ZTOK_LOCALE_BAREPORT, &childElement))
		{
			childElement.GetChildContents( m_szBAReportAddr, ZTOK_ADDR);
			childElement.GetChildContents( m_szBAReportDir,  ZTOK_DIR);
		}

		if (parentElement.FindChildNode(ZTOK_LOCALE_XMLHEADER, &childElement))
		{
			childElement.GetContents(m_Locale.szXmlHeader);
		}

		if (parentElement.FindChildNode(ZTOK_SKIN, &childElement))
		{
			childElement.GetContents(m_szInterfaceSkinName);
		}

		if (parentElement.FindChildNode(ZTOK_LOCALE_DEFFONT, &childElement))
		{
			childElement.GetContents(m_Locale.szDefaultFont);
		}

		if (parentElement.FindChildNode(ZTOK_LOCALE_IME, &childElement))
		{
 			childElement.GetContents(&m_Locale.bIMESupport);

			MEvent::SetIMESupport( m_Locale.bIMESupport);
		}
		if (parentElement.FindChildNode(ZTOK_LOCALE_HOMEPAGE, &childElement))
		{
			childElement.GetChildContents( m_Locale.szHomepageUrl,		ZTOK_LOCALE_HOMEPAGE_URL);
			childElement.GetChildContents( m_Locale.szHomepageTitle,	ZTOK_LOCALE_HOMEPAGE_TITLE);
		}
		if (parentElement.FindChildNode(ZTOK_LOCALE_EMBLEM_URL, &childElement))
		{
			childElement.GetContents( m_Locale.szEmblemURL);
		}
		if (parentElement.FindChildNode(ZTOK_LOCALE_CASHSHOP_URL, &childElement))
		{
			childElement.GetContents( m_Locale.szCashShopURL);
		}
		if (parentElement.FindChildNode(ZTOK_LOCATOR_LIST, &childElement))
		{
			m_pLocatorList->ParseLocatorList(childElement);
		}
		if (parentElement.FindChildNode(ZTOK_TLOCATOR_LIST, &childElement))
		{
			m_pTLocatorList->ParseLocatorList(childElement);
		}
	}
	xmlConfig.Destroy();

	m_bIsComplete = true;
	return true;
}

bool ZConfiguration::LoadConfig(const char* szFileName)
{
	MXmlDocument	xmlConfig;
	MXmlElement		parentElement, serverElement, bindsElement;
	MXmlElement		childElement;

	mlog( "Load Config from file : %s", szFileName );

	xmlConfig.Create();
	if (!xmlConfig.LoadFromFile(szFileName)) 
	{
		mlog( "- FAIL\n");
		xmlConfig.Destroy();
		return true;
	}
	mlog( "- SUCCESS\n");

	parentElement = xmlConfig.GetDocumentElement();
	int iCount = parentElement.GetChildNodeCount();

	if (!parentElement.IsEmpty())
	{
		if (parentElement.FindChildNode( ZTOK_SERVER, &serverElement))
		{
			serverElement.GetChildContents( m_szServerIP,	ZTOK_IP);
			serverElement.GetChildContents( &m_nServerPort,	ZTOK_PORT);
		}
		if (parentElement.FindChildNode(ZTOK_VIDEO, &childElement))
		{
			childElement.GetChildContents(&m_Video.nWidth, ZTOK_VIDEO_WIDTH);
			childElement.GetChildContents(&m_Video.nHeight, ZTOK_VIDEO_HEIGHT);
			childElement.GetChildContents(&m_Video.nColorBits, ZTOK_VIDEO_COLORBITS);

			char FullscreenMode[64];
			childElement.GetChildContents(FullscreenMode, ZTOK_VIDEO_FULLSCREEN);

			if (!_stricmp(FullscreenMode, "fullscreen"))
				m_Video.FullscreenMode = FullscreenType::Fullscreen;
			else if (!_stricmp(FullscreenMode, "borderless"))
				m_Video.FullscreenMode = FullscreenType::Borderless;
			else if (!_stricmp(FullscreenMode, "windowed"))
				m_Video.FullscreenMode = FullscreenType::Windowed;

			childElement.GetChildContents(&m_Video.nGamma, ZTOK_VIDEO_GAMMA);
			childElement.GetChildContents(&m_Video.bReflection,	ZTOK_VIDEO_REFLECTION );
			childElement.GetChildContents(&m_Video.bLightMap, ZTOK_VIDEO_LIGHTMAP );
			childElement.GetChildContents(&m_Video.bDynamicLight, ZTOK_VIDEO_DYNAMICLIGHT );
			childElement.GetChildContents(&m_Video.bShader, ZTOK_VIDEO_SHADER );
			childElement.GetChildContents(&m_Video.nCharTexLevel, ZTOK_VIDEO_CHARTEXLEVEL );
			childElement.GetChildContents(&m_Video.nMapTexLevel, ZTOK_VIDEO_MAPTEXLEVEL );
			childElement.GetChildContents(&m_Video.nEffectLevel, ZTOK_VIDEO_EFFECTLEVEL );
			childElement.GetChildContents(&m_Video.nTextureFormat, ZTOK_VIDEO_TEXTUREFORMAT );
			childElement.GetChildContents(&m_Video.bTerrible, "NHARDWARETNL");

			childElement.GetChildContents(&VisualFPSLimit, "VISUALFPSLIMIT");
			childElement.GetChildContents(&LogicalFPSLimit, "LOGICALFPSLIMIT");
			childElement.GetChildContents(&bCamFix, "CAMFIX");
			childElement.GetChildContents(&InterfaceFix, "INTERFACEFIX");
		}
		if (parentElement.FindChildNode(ZTOK_AUDIO, &childElement))
		{
			childElement.GetChildContents(&m_Audio.bBGMEnabled, ZTOK_AUDIO_BGM_ENABLED);
			childElement.GetChildContents(&m_Audio.fBGMVolume, ZTOK_AUDIO_BGM_VOLUME);
			childElement.GetChildContents(&m_Audio.fEffectVolume, ZTOK_AUDIO_EFFECT_VOLUME);
			childElement.GetChildContents(&m_Audio.bBGMMute, ZTOK_AUDIO_BGM_MUTE);
			childElement.GetChildContents(&m_Audio.bEffectMute, ZTOK_AUDIO_EFFECT_MUTE);
			childElement.GetChildContents(&m_Audio.b8BitSound, ZTOK_AUDIO_8BITSOUND);
			childElement.GetChildContents(&m_Audio.bInverse, ZTOK_AUDIO_INVERSE);
			childElement.GetChildContents(&m_Audio.bHWMixing, ZTOK_AUDIO_HWMIXING);
			childElement.GetChildContents(&m_Audio.bHitSound, ZTOK_AUDIO_HITSOUND);
			//childElement.GetChildContents(&m_Audio.b3DSound, ZTOK_AUDIO_3D_SOUND);
			m_Audio.b3DSound = true;
		}
		if (parentElement.FindChildNode(ZTOK_MOUSE, &childElement))
		{
			childElement.GetChildContents(&m_Mouse.fSensitivity, ZTOK_MOUSE_SENSITIVITY);
			childElement.GetChildContents(&m_Mouse.bInvert, ZTOK_MOUSE_INVERT);
		}
		if (parentElement.FindChildNode(ZTOK_JOYSTICK, &childElement))
		{
			childElement.GetChildContents(&m_Joystick.fSensitivity, ZTOK_JOYSTICK_SENSITIVITY);
		}
		if (parentElement.FindChildNode(ZTOK_KEYBOARD, &childElement))
		{
			for(int i=0; i<ZACTION_COUNT; i++){
				char szItemName[256];
				strcpy_safe(szItemName, m_Keyboard.ActionKeys[i].szName);
				_strupr_s(szItemName);

				MXmlNode keyNode;
				if (!childElement.FindChildNode(szItemName, &keyNode))
					continue;

				MXmlElement actionKeyElement = keyNode;

				int nKey;
				actionKeyElement.GetAttribute(&nKey,"alt",-1);
				if(nKey!=-1)
					m_Keyboard.ActionKeys[i].nVirtualKeyAlt = nKey;
				actionKeyElement.GetContents(&m_Keyboard.ActionKeys[i].nVirtualKey);
			}
		}

		if( parentElement.FindChildNode(ZTOK_MACRO, &childElement) )
		{
			childElement.GetChildContents(m_Macro.szMacro[0], ZTOK_MACRO_F1, 255);
			childElement.GetChildContents(m_Macro.szMacro[1], ZTOK_MACRO_F2, 255);
			childElement.GetChildContents(m_Macro.szMacro[2], ZTOK_MACRO_F3, 255);
			childElement.GetChildContents(m_Macro.szMacro[3], ZTOK_MACRO_F4, 255);
			childElement.GetChildContents(m_Macro.szMacro[4], ZTOK_MACRO_F5, 255);
			childElement.GetChildContents(m_Macro.szMacro[5], ZTOK_MACRO_F6, 255);
			childElement.GetChildContents(m_Macro.szMacro[6], ZTOK_MACRO_F7, 255);
			childElement.GetChildContents(m_Macro.szMacro[7], ZTOK_MACRO_F8, 255);
		}

		if (parentElement.FindChildNode(ZTOK_ETC, &childElement))
		{
			childElement.GetChildContents(&m_Etc.nNetworkPort1, ZTOK_ETC_NETWORKPORT1);
			childElement.GetChildContents(&m_Etc.nNetworkPort2, ZTOK_ETC_NETWORKPORT2);
			childElement.GetChildContents(&m_Etc.bBoost, ZTOK_ETC_BOOST);
			childElement.GetChildContents(&m_Etc.bRejectNormalChat, ZTOK_ETC_REJECT_NORMALCHAT);
			childElement.GetChildContents(&m_Etc.bRejectTeamChat, ZTOK_ETC_REJECT_TEAMCHAT);
			childElement.GetChildContents(&m_Etc.bRejectClanChat, ZTOK_ETC_REJECT_CLANCHAT);
			childElement.GetChildContents(&m_Etc.bRejectWhisper, ZTOK_ETC_REJECT_WHISPER);
			childElement.GetChildContents(&m_Etc.bRejectInvite, ZTOK_ETC_REJECT_INVITE);
			childElement.GetChildContents(&m_Etc.nCrossHair, ZTOK_ETC_CROSSHAIR);
			int temp{};
			childElement.GetChildContents(&temp, "DRAWTRAILS");
			bDrawTrails = temp != 0;
		}


		if (parentElement.FindChildNode("CHAT", &childElement))
		{
			char buf[64];
			childElement.GetChildContents(buf, "BACKGROUNDCOLOR");
			ChatBackgroundColor = strtoul(buf, nullptr, 16);
		}
	}

	xmlConfig.Destroy();

	return true;
}

struct ConfigSection
{
	MXmlElement Element;
	bool IsFirst = true;

	ConfigSection(MXmlElement& RootElement, const char* Name) :
		Element{ RootElement.CreateChildElement(Name) }
	{
		Element.AppendText("\n\t\t");
	}

	~ConfigSection()
	{
		Element.AppendText("\n\t");
	}

	// Non-floating point overload
	template <typename T>
	std::enable_if_t<!std::is_floating_point<T>::value, MXmlElement> Add(const char* Name, T Value)
	{
		if (!IsFirst) {
			Element.AppendText("\n\t\t");
		} else {
			IsFirst = false;
		}

		auto NewElement = Element.CreateChildElement(Name);
		NewElement.SetContents(Value);
		return NewElement;
	}

	// Floating point overload
	// The default MXmlElement::SetContents for float has weird padding, so we'll do our own thing.
	template <typename T>
	std::enable_if_t<std::is_floating_point<T>::value, MXmlElement> Add(const char* Name, T Value)
	{
		char buf[128];
		sprintf_safe(buf, "%f", Value);
		return Add(Name, buf);
	}

	auto AddHex(const char* Name, u32 Value)
	{
		char buf[64];
		sprintf_safe(buf, "%08X", Value);
		return Add(Name, buf);
	}
};

static const char* GetFullscreenString(FullscreenType Mode)
{
	switch (Mode)
	{
	case FullscreenType::Fullscreen:
		return "fullscreen";
	case FullscreenType::Borderless:
		return "borderless";
	case FullscreenType::Windowed:
		return "windowed";
	default:
		assert(false);
		return "fullscreen";
	};
}

bool ZConfiguration::SaveToFile(const char *szFileName, const char* szHeader)
{
	MXmlDocument xmlConfig;
	xmlConfig.Create();
	xmlConfig.CreateProcessingInstruction(szHeader);

	auto RootElement = xmlConfig.CreateElement("XML");
	RootElement.AppendText("\n\t");

	xmlConfig.AppendChild(RootElement);

	auto AddIntersectionWhitespace = [&] {
		RootElement.AppendText("\n\n\t");
	};

	// Server
	{
		auto Section = ConfigSection(RootElement, ZTOK_SERVER);
		Section.Add(ZTOK_IP, m_szServerIP);
		Section.Add(ZTOK_PORT, m_nServerPort);
	}

	AddIntersectionWhitespace();

	// Skin
	{
		auto skinElement = RootElement.CreateChildElement(ZTOK_SKIN);
		skinElement.SetContents(m_szInterfaceSkinName);
		skinElement.AppendText("");
	}

	AddIntersectionWhitespace();

	// Video
	{
		auto Section = ConfigSection(RootElement, ZTOK_VIDEO);
		Section.Add(ZTOK_VIDEO_WIDTH, m_Video.nWidth);
		Section.Add(ZTOK_VIDEO_HEIGHT, m_Video.nHeight);
		Section.Add(ZTOK_VIDEO_COLORBITS, m_Video.nColorBits);
		Section.Add(ZTOK_VIDEO_GAMMA, m_Video.nGamma);
		Section.Add(ZTOK_VIDEO_FULLSCREEN, GetFullscreenString(m_Video.FullscreenMode));
		Section.Add(ZTOK_VIDEO_REFLECTION, m_Video.bReflection);
		Section.Add(ZTOK_VIDEO_LIGHTMAP, m_Video.bLightMap);
		Section.Add(ZTOK_VIDEO_DYNAMICLIGHT, m_Video.bDynamicLight);
		Section.Add(ZTOK_VIDEO_SHADER, m_Video.bShader);
		Section.Add(ZTOK_VIDEO_CHARTEXLEVEL, m_Video.nCharTexLevel);
		Section.Add(ZTOK_VIDEO_MAPTEXLEVEL, m_Video.nMapTexLevel);
		Section.Add(ZTOK_VIDEO_EFFECTLEVEL, m_Video.nEffectLevel);
		Section.Add(ZTOK_VIDEO_TEXTUREFORMAT, m_Video.nTextureFormat);
		Section.Add("NHARDWARETNL", m_Video.bTerrible);
		Section.Add("VISUALFPSLIMIT", VisualFPSLimit);
		Section.Add("LOGICALFPSLIMIT", LogicalFPSLimit);
		Section.Add("CAMFIX", bCamFix);
		Section.Add("INTERFACEFIX", InterfaceFix);
	}

	AddIntersectionWhitespace();

	// Audio
	{
		auto Section = ConfigSection(RootElement, ZTOK_AUDIO);
		Section.Add(ZTOK_AUDIO_BGM_MUTE, m_Audio.bBGMMute);
		Section.Add(ZTOK_AUDIO_BGM_VOLUME, m_Audio.fBGMVolume);
		Section.Add(ZTOK_AUDIO_EFFECT_VOLUME, m_Audio.fEffectVolume);
		Section.Add(ZTOK_AUDIO_3D_SOUND, m_Audio.b3DSound);
		Section.Add(ZTOK_AUDIO_8BITSOUND, m_Audio.b8BitSound);
		Section.Add(ZTOK_AUDIO_INVERSE, m_Audio.bInverse);
		Section.Add(ZTOK_AUDIO_HWMIXING, m_Audio.bHWMixing);
		Section.Add(ZTOK_AUDIO_HITSOUND, m_Audio.bHitSound);
	}

	AddIntersectionWhitespace();

	// Mouse
	{
		auto Section = ConfigSection(RootElement, ZTOK_MOUSE);
		Section.Add(ZTOK_MOUSE_SENSITIVITY, m_Mouse.fSensitivity);
		Section.Add(ZTOK_MOUSE_INVERT, m_Mouse.bInvert);
	}

	AddIntersectionWhitespace();

	// Joystick
	{
		auto Section = ConfigSection(RootElement, ZTOK_JOYSTICK);
		Section.Add(ZTOK_JOYSTICK_SENSITIVITY, m_Joystick.fSensitivity);
	}

	AddIntersectionWhitespace();

	// Control
	{
		auto Section = ConfigSection(RootElement, ZTOK_KEYBOARD);
		
		for (auto&& ActionKey : m_Keyboard.ActionKeys)
		{
			if (ActionKey.szName[0] == 0)
				continue;

			char szItemName[256];
			strcpy_safe(szItemName, ActionKey.szName);
			_strupr_s(szItemName);

			auto KeyElement = Section.Add(szItemName, ActionKey.nVirtualKey);
			KeyElement.SetAttribute("alt", ActionKey.nVirtualKeyAlt);
		}
	}

	AddIntersectionWhitespace();

	// Macro
	{
		auto Section = ConfigSection(RootElement, ZTOK_MACRO);

		const char* MacroStrings[ZCONFIG_MACRO_MAX] = {
			ZTOK_MACRO_F1,
			ZTOK_MACRO_F2,
			ZTOK_MACRO_F3,
			ZTOK_MACRO_F4,
			ZTOK_MACRO_F5,
			ZTOK_MACRO_F6,
			ZTOK_MACRO_F7,
			ZTOK_MACRO_F8
		};

		for (int i = 0; i < int(std::size(MacroStrings)); ++i)
		{
			Section.Add(MacroStrings[i], m_Macro.szMacro[i]);
		}
	}

	AddIntersectionWhitespace();

	// Etc
	{
		auto Section = ConfigSection(RootElement, ZTOK_ETC);
		Section.Add(ZTOK_ETC_NETWORKPORT1, m_Etc.nNetworkPort1);
		Section.Add(ZTOK_ETC_NETWORKPORT2, m_Etc.nNetworkPort2);
		Section.Add(ZTOK_ETC_BOOST, m_Etc.bBoost);
		Section.Add(ZTOK_ETC_REJECT_NORMALCHAT, m_Etc.bRejectNormalChat);
		Section.Add(ZTOK_ETC_REJECT_TEAMCHAT, m_Etc.bRejectTeamChat);
		Section.Add(ZTOK_ETC_REJECT_CLANCHAT, m_Etc.bRejectClanChat);
		Section.Add(ZTOK_ETC_REJECT_WHISPER, m_Etc.bRejectWhisper);
		Section.Add(ZTOK_ETC_REJECT_INVITE, m_Etc.bRejectInvite);
		Section.Add(ZTOK_ETC_CROSSHAIR, m_Etc.nCrossHair);
		Section.Add("DRAWTRAILS", bDrawTrails);
	}

	AddIntersectionWhitespace();

	// Chat
	{
		auto Section = ConfigSection(RootElement, "CHAT");
		Section.AddHex("BACKGROUNDCOLOR", ChatBackgroundColor);
	}

	RootElement.AppendText("\n");

	return xmlConfig.SaveToFile(szFileName);
}

void ZConfiguration::Init()
{
	m_Video.FullscreenMode = FullscreenType::Fullscreen;
	auto Width = GetSystemMetrics(SM_CXSCREEN);
	if (Width == 0)
		Width = 1024;
	auto Height = GetSystemMetrics(SM_CYSCREEN);
	if (Height == 0)
		Height = 768;
	m_Video.nWidth = Width;
	m_Video.nHeight = Height;
	m_Video.nColorBits = 32;
	m_Video.nGamma = 255;
	m_Video.bShader		= true;
	m_Video.bLightMap	= false;
	m_Video.bReflection	= true;
	// 0 = high
	m_Video.nCharTexLevel = 0;
	m_Video.nMapTexLevel = 0;
	m_Video.nEffectLevel = Z_VIDEO_EFFECT_HIGH;
	m_Video.nTextureFormat = 1;
	m_Video.bTerrible = false;
	
	m_Audio.bBGMEnabled = true;
	m_Audio.fBGMVolume	= 0.3f;
	m_Audio.bBGMMute	= false;
	m_Audio.bEffectMute = false;
	m_Audio.fEffectVolume = 0.3f;
	m_Audio.b3DSound	= true;
	m_Audio.b8BitSound	= false;
	m_Audio.bInverse	= false;
	m_Audio.bHWMixing	= false;
	m_Audio.bHitSound	= true;

	m_Mouse.fSensitivity = 1.f;
	m_Mouse.bInvert = false;

	m_Joystick.fSensitivity = 1.f;
	m_Joystick.bInvert = false;

	for (int i = 0; i < int(std::size(m_Macro.szMacro)); ++i)
		sprintf_safe(m_Macro.szMacro[i], "STR:CONFIG_MACRO_F%d", i + 1);

	m_Etc.nNetworkPort1 = 7700;
	m_Etc.nNetworkPort2 = 7800;
	m_Etc.nCrossHair = 0;
	m_Etc.bInGameNoChat = false;

	m_bOptimization = false;
	
	memset(m_szServerIP, 0, sizeof(m_szServerIP));
	strcpy_safe(m_szServerIP, "127.0.0.1");
	m_nServerPort = 6000;
	strcpy_safe(m_szInterfaceSkinName, DEFAULT_INTERFACE_SKIN);

	LoadDefaultKeySetting();

	strcpy_safe(m_Locale.szDefaultFont, "Arial");
	strcpy_safe(m_Locale.szXmlHeader, "version=\"1.0\" encoding=\"UTF-8\"");
	m_Locale.szHomepageUrl[0] = 0;
	m_Locale.szHomepageTitle[0] = 0;
	strcpy_safe(m_Locale.szEmblemURL, "");
	strcpy_safe(m_Locale.szCashShopURL, "http://www.gunzonline.com/");
	m_Locale.bIMESupport = false;

	m_bViewGameChat = true;
}

void ZConfiguration::LoadDefaultKeySetting()
{
	static ZACTIONKEYDESCRIPTION DefaultActionKeys[ZACTION_COUNT] = {

		{"Forward",		0x11, -1},	// 'w'
		{"Back",		0x1f, -1},	// 's'
		{"Left",		0x1e, -1},	// 'a'
		{"Right",		0x20, -1},	// 'd'

		{"MeleeWeapon", 0x02, -1},	// '1'
		{"PrimaryWeapon",0x03, -1},	// '2'
		{"SecondaryWeapon",0x04, -1},// '3'
		{"Item1",		0x05, -1},	// '4'
		{"Item2",		0x06, -1},	// '5'

		{"PrevousWeapon",0x10, -1},	// 'q'
		{"NextWeapon", 0x12,-1},	// 'e'
		{"Reload",		0x13,-1},	// 'r'
		{"Jump",		0x39,-1},	// space
		{"Score",		0x0f,-1},	// tab
		{"Menu",		0x01,-1},	// esc

		{"Taunt",		0x29,-1},	// '`'
		{"Bow",			0x07,-1},	// '6'
		{"Wave",		0x08,-1},	// '7'
		{"Laugh",		0x09,-1},	// '8'
		{"Cry",			0x0a,-1},	// '9'
		{"Dance",		0x0b,-1},	// '0'

		{"ScreenShot",	0x58,-1},	// 'F12'
		{"Record",		0x57,-1},	// 'F11'
		{"Defence",		0x2a,-1},	// 'shift'
		{"ToggleChat",	0x2f,-1},	// 'v'

		{"UseWeapon",	0x1D,258},	// 'ctrl' or	mouse LButton
		{"UseWeapon2", 259, -1},		// mouse RButton
		{ "ShowFullChat", 44, -1 },
		{ "VoiceChat", 37, -1 },
		// Ãß°¡ by Á¤µ¿¼· @ 2006/3/16
	};

	_ASSERT(ZACTION_COUNT==sizeof(DefaultActionKeys)/sizeof(ZACTIONKEYDESCRIPTION));

	memcpy(m_Keyboard.ActionKeys, DefaultActionKeys, sizeof(ZACTIONKEYDESCRIPTION)*ZACTION_COUNT);
}

ZSERVERNODE ZConfiguration::GetServerNode( int nNum) const
{
	ZSERVERNODE ServerNode;

	map<int,ZSERVERNODE>::const_iterator iterator;

	iterator = m_ServerList.find( nNum);
	if ( iterator != m_ServerList.end())
	{
		ServerNode = (*iterator).second;
	}

	return ServerNode;
}


bool ZConfiguration::ConvertMacroStrings()
{
	for (int i = 0; i < 8; i++)
	{
		auto&& Macro = m_Macro.szMacro[i];
		if (strncmp(Macro, "STR:", 4) == 0)
			strcpy_safe(Macro, ZGetStringResManager()->GetStringFromXml(Macro));
	}

	return true;
}
