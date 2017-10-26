cmake_policy(SET CMP0015 NEW)

link_directories(
	../Gunz
	../sdk/zlib/lib
	../sdk/dx9/lib/x86
	../RealSound/Lib
	../sdk/opus/lib/Release
	../sdk/portaudio/lib/Release)
	
file(GLOB src
    "./*.h"
    "./*.cpp"
)

add_target(NAME Tests TYPE EXECUTABLE SOURCES "${src}")

target_include_directories(Tests PRIVATE
	../cml/include
	../CSCommon/include
	../Mint2/include
	../SafeUDP/include
	../RealSpace2/include
	../Gunz
	../launcher/src
	../sdk/dx9/include
	../sdk/opus/include
	../sdk/portaudio/include)

find_library(d3dx9_path NAMES d3dx9 PATHS ../sdk/dx9/lib/x86)
find_library(dxerr_path NAMES dxerr PATHS ../sdk/dx9/lib/x86)

target_link_libraries(Tests PRIVATE
	cml
	RealSpace2
	Gunz_lib
	launcher_lib
	sodium
	rapidxml
	opus
	${d3dx9_path}
	${dxerr_path}
	legacy_stdio_definitions.lib)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	ucm_add_linker_flags(/ignore:4099)
endif()