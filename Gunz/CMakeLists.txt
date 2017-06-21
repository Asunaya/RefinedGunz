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

set(src "${src};${CMAKE_CURRENT_SOURCE_DIR}/Gunz.rc")

add_target(NAME Gunz TYPE EXECUTABLE SOURCES "${src}")
add_target(NAME Gunz_lib TYPE STATIC SOURCES "${src}")

macro(target_include_directories_both)
	target_include_directories(Gunz ${ARGV})
	target_include_directories(Gunz_lib ${ARGV})
endmacro()

macro(target_link_libraries_both)
	target_link_libraries(Gunz ${ARGV})
	target_link_libraries(Gunz_lib ${ARGV})
endmacro()

macro(add_dependencies_both)
	add_dependencies(Gunz ${ARGV})
	add_dependencies(Gunz_lib ${ARGV})
endmacro()

macro(target_compile_options_both)
	target_compile_options(Gunz ${ARGV})
	target_compile_options(Gunz_lib ${ARGV})
endmacro()

target_include_directories_both(PRIVATE
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
	../sdk/portaudio/include)

find_library(zlib_path NAMES zlib PATHS ../sdk/zlib/lib)
find_library(fmodvc_path NAMES fmodvc PATHS .)

find_library(libsodium_path NAMES libsodium PATHS ../sdk/libsodium/lib/static)

target_link_libraries_both(PUBLIC
	${zlib_path}
	${fmodvc_path}
	${libsodium_path}
	legacy_stdio_definitions.lib
	cml
	RealSpace2
	CSCommon
	Mint2
	RealSound
	SafeUDP
	MDatabase)

add_dependencies_both(cml RealSpace2 CSCommon MDatabase Mint2 RealSound SafeUDP)

target_compile_options_both(PRIVATE 
		-DSODIUM_STATIC)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	target_compile_options_both(PRIVATE
		/wd4244 /wd4305)
	ucm_add_linker_flags(
		/ignore:4099 /ignore:4221
		/SUBSYSTEM:WINDOWS)
endif()