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
	../sdk/rapidxml/include
	../sdk/libsodium/include
	../sdk/dx9/include
	../sdk/opus/include
	../sdk/portaudio/include)

add_dependencies(Tests Gunz_lib)
add_dependencies(Tests launcher_lib)

find_library(libsodium_path NAMES libsodium PATHS ../sdk/libsodium/lib/static)
add_definitions(-DSODIUM_STATIC)

find_library(celt_path NAMES celt PATHS ../sdk/opus/lib/Release)
find_library(opus_path NAMES opus PATHS ../sdk/opus/lib/Release)
find_library(silk_common_path NAMES silk_common PATHS ../sdk/opus/lib/Release)
find_library(silk_fixed_path NAMES silk_fixed PATHS ../sdk/opus/lib/Release)
find_library(silk_float_path NAMES silk_float PATHS ../sdk/opus/lib/Release)

find_library(d3dx9_path NAMES d3dx9 PATHS ../sdk/dx9/lib/x86)
find_library(dxerr_path NAMES dxerr PATHS ../sdk/dx9/lib/x86)

target_link_libraries(Tests PRIVATE
	cml
	RealSpace2
	Gunz_lib
	launcher_lib
	${libsodium_path}
	${celt_path}
	${opus_path}
	${silk_common_path}
	${silk_fixed_path}
	${silk_float_path}
	${d3dx9_path}
	${dxerr_path}
	legacy_stdio_definitions.lib)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	ucm_add_linker_flags(/ignore:4099)
endif()