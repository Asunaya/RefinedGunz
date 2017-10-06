# Table of Contents

- [Refined Gunz](#refined-gunz)
- [Usage](#usage)
- [Building](#building)
	- [Building shaders](#building-shaders)
	- [Configuration](#configuration)
- [Features](#features)
	- [Gameplay](#gameplay)
		- [Gladiator changes](#gladiator-changes)
	- [New gamemodes](#new-gamemodes)
	- [Replays](#replays)
	- [Misc client stuff](#misc-client-stuff)
	- [Server](#server)
- [Credits](#credits)

# Refined Gunz
Refined Gunz is a modified version of Gunz the Duel, a game by MAIET Entertainment. It's based on the leaked Gunz 1.0 source from around 2007.

# Usage
To start up a basic local server and play on that, download binaries from the [Releases](https://github.com/Asunaya/RefinedGunz/releases) page on here, run the server (MatchServer.exe) and then run the client (Gunz.exe). There is no additional configuration required.

To connect to someone else's server instance, change the IP in config.xml to their IP or domain. (Make sure ports are open, etc.)

Running the client requires DirectX 9 (or Vulkan support, although that is unfinished). There are no other external dependencies.

Running the server with SQLite (the default) requires no external dependencies or configuration. Running it with MSSQL requires you to compile it with MSSQL support (pass `-DMSSQL=1` to cmake), set up ODBC and a matching MSSQL server instance, and set up database login info in server.ini.

Note that since Refined Gunz is based on Gunz 1.0, MSSQL database files, XMLs, etc. from Gunz 1.5 distributions may not work with it.

# Building
Requisites:

* CMake 3.7 or later. You can download the latest release from https://cmake.org/download/ under "Binary distributions." In the installer, select "Add CMake to the system PATH for all users." [Picture here](https://i.imgur.com/rQHLXX8.png).
* Visual Studio 2017 with Windows XP support and ATL/MFC support. You can download the free Community edition at https://www.visualstudio.com/downloads/. In the installer, select the "Desktop development with C++," and on the right under "Summary," select "Windows XP support for C++" and "MFC and ATL support (x86 and x64)." [Picture here](https://i.imgur.com/BqXoiXu.png)).

To build, enter the "msvcbuild" folder in the repository, and run "build.bat".

## Building shaders
Shaders are already precompiled, but the Visual Studio solutions do not recompile them if you edit them. To recompile the shaders, run the relevant .bat file in the RealSpace2 folder.

BuildHLSLShaders.bat builds the HLSL shaders for D3D9. It requires the fxc compiler which can be found in the DirectX 9 SDK. The path to it is hardcoded in the .bat; change the %fxc% variable if it doesn't match.

BuildGLSLShaders.bat builds the GLSL shaders for Vulkan. It requires the glslang compiler (glslangvalidator) which can be found in the Vulkan SDK.

## Configuration
Features of the server can be configured in the file "CSCommon/config.h".

# Features

## Gameplay
- 3 netcode modes:
 - Server-based -- (Unfinished) No direct connections are established, hit registration is done with lag compensation on the server. Only enabled if game_dir in server.xml points to a valid game directory that contains animation and map files
 - Peer to Peer Antilead -- Peer to peer, attacker does immediate hit registration
 - Peer to Peer Lead -- Peer to peer, target does immediate hit registration
- Voice chat
- Passwords hashed on the client with BLAKE2b and on the server with salted scrypt
- Remade chat -- Resizable, movable, copyable text, unlimited history, transparent background
- Basic info update rate per second increased from 10 to 33
- Spectator mode
- Portals
- Commands -- See [the wiki list](https://github.com/Asunaya/RefinedGunz/wiki/Chat-commands)

### Gladiator changes:
- See [Gladiator changes](https://github.com/Asunaya/RefinedGunz/wiki/Gladiator-changes)

## New gamemodes
- Skillmap -- Skillmaps contain start and end zones, track your time, and teleport you back to the start zone when you fall. Will have a time record list in the future. Other players are partially transparent and cannot damage or push you.
- Gun game -- A gamemode where your equipped weapons change every time you get a kill.

## Replays
- Runs multiple formats -- Have so far tested Official V4, V6, V7, V10 and V11; Freestyle Gunz V7 (both formats), V8, V9 and V10; and Dark Gunz V6. If you'd like another replay format to be supported, [make an issue](https://github.com/Asunaya/RefinedGunz/issues/new) with the replay file.
- Seek bar to jump to different time locations
- Data in the replay list -- Shows gametype, map name, stage name, player names, scores, etc.

## Misc client stuff
- Registration at the login screen
- Supports fullscreen, borderless and windowed mode
- Dynamic resource loading -- Clothes are loaded individually as they are required, not loaded all together when you start the game. This reduces load time and memory usage.
- Supports Gunz 2/RS3 maps

## Server
- MatchServer supports SQLite in addition to MSSQL
- MatchServer comprises the previous modules (MatchAgent, Locator) by default, although you can run them freestanding as well

# Credits
- grandao for http://forum.ragezone.com/f245/release-development-rsx-realspacex-1060866/
- Jetman for gun game gamemode