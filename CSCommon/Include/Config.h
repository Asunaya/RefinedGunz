#pragma once

// Controls whether voicechat is enabled, and whether libraries this feature
// depends on (portaudio, Opus) are linked into the Gunz executable.
#define VOICECHAT

// Controls whether portals can be created.
#define PORTAL

// Controls whether rockets can be reflected by blocking them.
//#define REFLECT_ROCKETS

// Controls whether attacks can be blocked in the guard_start animation.
//#define GUARD_START_CAN_BLOCK

// Controls whether aiming at a player will change the crosshair to a different pick bitmap.
// This allows for the creation of extremely simple triggerbots, so it's off by default.
//#define CROSSHAIR_PICK

// Controls whether the /timescale command can be used in developer mode.
#define TIMESCALE

// Controls whether the locator IP(s) are loaded from config.xml or system.mrs/system.xml
#define LOAD_LOCATOR_FROM_CONFIG_XML

// The interval between client transmission of MC_PEER_BASICINFO packets in milliseconds.
// Basicinfo packets contain several core pieces of physical information:
// Position, direction, velocity, animation indices and selected item slot.
#define BASICINFO_INTERVAL 10 // Milliseconds

// Controls whether your camera direction is locked to a small 90 degree area
// when dashing, hanging, etc. See ZMyCharacter::IsDirLocked for a full list.
#define NO_DIRECTION_LOCK

// Default field of view in radians.
#define DEFAULT_FOV		1.22173048f // 70 degrees in radians

// Default distance from the camera to the near plane in the projection transform.
#define DEFAULT_NEAR_Z	5.0f

// Default distance from the camera to the far plane in the projection transform.
#define DEFAULT_FAR_Z	100000.0f

// Controls whether only MRS files will be loaded in release mode
//#define ONLY_LOAD_MRS_FILES

// Controls whether every map in the maps folder will be added
// to the ingame maps list regardless of whether it's in g_MapDesc or not
#define ADD_ALL_MAPS