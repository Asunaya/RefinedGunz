cmake_policy(SET CMP0015 NEW)

link_directories(
	../Gunz
	../sdk/zlib/lib
	../sdk/dx9/lib/x86
	../RealSound/Lib
	../sdk/opus/lib/Release
	../sdk/portaudio/lib/Release)

file(GLOB src
    "*.h"
    "*.cpp"
)

set(main_file "${CMAKE_CURRENT_SOURCE_DIR}/WinMain.cpp")
list(REMOVE_ITEM src "${main_file}")

add_target(NAME Gunz_lib TYPE STATIC SOURCES "${src}")

target_include_directories(Gunz_lib
PUBLIC
	../sdk/dx9/include
	../sdk
	../CSCommon/Include
	../SafeUDP/Include
	../CML/Include
	../RealSpace2/Include
	../MAI/Include
	../RealSpace/Include
	../Mint2/Include
	../RealSound/Include
	../MDatabase/Include
	../MCountryFilter/Include
	../sdk/rapidxml/include
	../sdk/libsodium/include
	../sdk/opus/include
	../sdk/portaudio/include
INTERFACE
    .
)

find_library(zlib_path NAMES zlib PATHS ../sdk/zlib/lib)
find_library(fmodvc_path NAMES fmodvc PATHS .)

find_library(libsodium_path NAMES libsodium PATHS ../sdk/libsodium/lib/static)

target_link_libraries(Gunz_lib PUBLIC
	legacy_stdio_definitions.lib
	winmm.lib
	${zlib_path}
	${fmodvc_path}
	sodium
	opus
	bullet
    portaudio
	
	cml
	RealSpace2
	CSCommon
	Mint2
	RealSound
	SafeUDP
	MDatabase
)

target_compile_options(Gunz_lib
    PUBLIC -D_LOCATOR
    PRIVATE -DLIB_COMPILE)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	target_compile_options(Gunz_lib PUBLIC
		/wd4244 /wd4305)
	ucm_add_linker_flags(/SUBSYSTEM:WINDOWS)
endif()

add_target(NAME Gunz TYPE EXECUTABLE SOURCES "${main_file};${CMAKE_CURRENT_SOURCE_DIR}/Gunz.rc")
target_link_libraries(Gunz PUBLIC Gunz_lib)
