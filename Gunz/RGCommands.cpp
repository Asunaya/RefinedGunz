#include "stdafx.h"
#include "ZConfiguration.h"
#include "NewChat.h"
#include "RGMain.h"
#include "VoiceChat.h"
#include "Extensions.h"

void LoadRGCommands(ZChatCmdManager &CmdManager)
{
	CmdManager.AddCommand(0, "fpslimit", [](const char *line, int argc, char ** const argv){
		int nFPSLimit = atoi(argv[1]);
		ZGetConfiguration()->nFPSLimit = nFPSLimit;
		ZChatOutputF("FPS limit set to %d", nFPSLimit);

		ZGetConfiguration()->Save();
	},
		CCF_ALL, 1, 1, true, "/fpslimit <fps>", "");


	CmdManager.AddCommand(0, "camfix", [](const char *line, int argc, char ** const argv){
		bool bCamFix;
		if (argc == 1)
			bCamFix = !ZGetConfiguration()->GetCamFix();
		else
			bCamFix = atoi(argv[1]) != 0;

		ZGetConfiguration()->bCamFix = bCamFix;
		if (bCamFix)
			ZGetCamera()->m_fDist = CAMERA_DEFAULT_DISTANCE * RGetScreenWidth() / RGetScreenHeight() / (4.f / 3.f);
		else
			ZGetCamera()->m_fDist = CAMERA_DEFAULT_DISTANCE;

		ZGetConfiguration()->Save();

		ZChatOutputF("Cam fix %s", bCamFix ? "enabled" : "disabled");
	},
		CCF_ALL, 0, 1, true, "/camfix [0/1]", "");


	CmdManager.AddCommand(0, "backgroundcolor", [](const char *line, int argc, char ** const argv){
		DWORD BackgroundColor = strtoul(argv[1], NULL, 16);
		g_Chat.SetBackgroundColor(BackgroundColor);
		ZGetConfiguration()->ChatBackgroundColor = BackgroundColor;

		ZGetConfiguration()->Save();

		ZChatOutputF("Background color set to %08X", BackgroundColor);
	},
		CCF_ALL, 1, 1, true, "/backgroundcolor <AARRGGBB hex color>", "");


	CmdManager.AddCommand(0, "replayseek",
		[](const char *line, int argc, char ** const argv){
		ZGetGame()->SetReplayTime(atof(argv[1]));
	},
		CCF_ALL, 1, 1, true, "/replayseek <time>", "");


	CmdManager.AddCommand(0, "spec", [](const char *line, int argc, char ** const argv){
		if (ZGetGameInterface()->GetState() != GUNZ_GAME)
			return;

		ZCharacter &mychar = *ZGetGame()->m_pMyCharacter;
		if (mychar.GetTeamID() == MMT_SPECTATOR)
		{
			ZChatOutputF("Spectator mode is already enabled");
			return;
		}

		mychar.SetTeamID(MMT_SPECTATOR);
		mychar.ForceDie();
		ZPostStageTeam(ZGetGame()->m_pMyCharacter->GetUID(), ZGetGameClient()->GetStageUID(), MMT_SPECTATOR);
		ZPostSpec();

		ZChatOutputF("Spectator mode enabled.");
	}, CCF_ALL, 0, 0, true, "/spec", "");

	CmdManager.AddCommand(0, "fullscreen", [](const char *line, int argc, char ** const argv){
		ZGetConfiguration()->GetVideo()->bFullScreen = !ZGetConfiguration()->GetVideo()->bFullScreen;

		RMODEPARAMS ModeParams = { RGetScreenWidth(), RGetScreenHeight(), ZGetConfiguration()->GetVideo()->bFullScreen, RGetPixelFormat() };

		RResetDevice(&ModeParams);

		RAdjustWindow(&ModeParams);

		ZGetConfiguration()->Save();
	}, CCF_ALL, 0, 0, true, "/fullscreen", "");

	CmdManager.AddCommand(0, "setparts", [](const char *line, int argc, char ** const argv){
		ZGetGame()->m_pMyCharacter->m_pVMesh->SetParts((RMeshPartsType)atoi(argv[1]), argv[2]);
	}, CCF_ALL, 0, 0, true, "/fullscreen", "");

	auto Sens = [](const char *line, int argc, char ** const argv) {
		float fSens = atof(argv[1]);

		ZGetConfiguration()->GetMouse()->fSensitivity = fSens;

		ZGetConfiguration()->Save();

		ZChatOutputF("Sensitivity set to %f", fSens);
	};

	CmdManager.AddCommand(0, "sensitivity", Sens, CCF_ALL, 1, 1, true, "/sensitivity <value>", "");
	CmdManager.AddCommand(0, "sens", Sens, CCF_ALL, 1, 1, true, "/sens <value>", "");

	CmdManager.AddCommand(0, "admin_mute", [](const char *line, int argc, char ** const argv){
		ZPostAdminMute(argv[1], argv[3] ? argv[3] : "", atoi(argv[3]) * 60);
	}, CCF_ADMIN, 2, 3, true, "/admin_mute <name> <minutes> [reason]", "");

	CmdManager.AddCommand(0, "argv", [](const char *line, int argc, char ** const argv) {
		for (int i = 0; i < argc; i++)
		{
			ZChatOutputF("%s", argv[i]);
		}
	}, CCF_ALL, ARGVNoMin, ARGVNoMax, true, "/argv", "");

	CmdManager.AddCommand(0, "swordcolor", [](const char *line, int argc, char ** const argv) {
		uint32_t Color = strtoul(argv[1], NULL, 16);
		ZPOSTCMD1(MC_PEER_SET_SWORD_COLOR, MCmdParamUInt(Color));
	}, CCF_ALL, 1, 1, true, "/swordcolor <AARRGGBB>", "");

#ifdef VOICECHAT
	CmdManager.AddCommand(0, "mute", [](const char *line, int argc, char ** const argv) {
		auto ret = FindSinglePlayer(argv[1]);

		if (!ret.second)
		{
			switch (ret.first)
			{
			case -1:
				ZChatOutputF("No player with %s in their name was found", argv[1]);
				break;

			case -2:
				ZChatOutputF("Too many players with %s in their name was found", argv[1]);
				break;

			default:
				ZChatOutputF("Unknown error");
			};
			
			return;
		}

		bool b = g_RGMain->MutePlayer(ret.second->GetUID());

		ZChatOutputF("%s has been %s", ret.second->GetUserNameA(), b ? "muted" : "unmuted");
	}, CCF_ALL, 1, 1, true, "/swordcolor <AARRGGBB>", "");
#endif

	CmdManager.AddCommand(0, "debug", [](const char *line, int argc, char ** const argv) {
		ZGetConfiguration()->HitRegistrationDebugOutput = !ZGetConfiguration()->HitRegistrationDebugOutput;
		ZGetConfiguration()->Save();

		ZGetGameClient()->ClientSettings.DebugOutput = ZGetConfiguration()->HitRegistrationDebugOutput;
		ZPostClientSettings(ZGetGameClient()->ClientSettings);

		ZChatOutputF("Debug output %s",
			ZGetConfiguration()->HitRegistrationDebugOutput ? "enabled" : "disabled");
	},
		CCF_ALL, 0, 0, true, "/debug", "");

	CmdManager.AddCommand(0, "hello", [](const char *line, int argc, char ** const argv) {
		ZChatOutput("Hi! ^__^", ZChat::CMT_SYSTEM, ZChat::CL_CURRENT);
	},
		CCF_ALL, 0, 0, true, "/hello", "");

	CmdManager.AddCommand(0, "timescale", [](const char *line, int argc, char ** const argv) {
		if (ZApplication::GetInstance()->GetLaunchMode() != ZApplication::ZLAUNCH_MODE_STANDALONE_GAME)
		{
			ZChatOutput("Timescale can only be set in debug mode");
			return;
		}

		extern float g_Timescale;
		float NewTimescale = atof(argv[1]);
		g_Timescale = NewTimescale;

		ZChatOutputF("Set timescale to %f, %s.", NewTimescale, argv[1]);
	},
		CCF_ALL, 1, 1, true, "/timescale <scale>", "");
}

#ifdef TIMESCALE
unsigned long long GetGlobalTimeMS()
{
	if (!ZApplication::GetInstance())
		return timeGetTime();

	return ZApplication::GetInstance()->GetTime();
}
#endif