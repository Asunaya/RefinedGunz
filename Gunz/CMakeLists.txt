cmake_minimum_required(VERSION 2.8)
include(../cmake/macros.cmake)
cmake_policy(SET CMP0015 OLD)

project(Gunz CXX)

link_directories(../Gunz;../sdk/zlib/lib;../sdk/dx9/lib/x86;../RealSound/Lib;../sdk/opus/lib/Release;../sdk/portaudio/lib/Release)

file(GLOB src
    "*.h"
    "*.cpp"
)

set(src "${src};${CMAKE_CURRENT_SOURCE_DIR}/Gunz.rc")

add_target(NAME Gunz TYPE EXECUTABLE SOURCES "${src}" PCH_FILE stdafx.h)

target_include_directories(Gunz PUBLIC ../sdk/dx9/include;../sdk;../CSCommon/Include;../SafeUDP/Include;../CML/Include;../RealSpace2/Include;../MAI/Include;../RealSpace/Include;../Mint2/Include;../RealSound/Include;../MDatabase/Include;../MCountryFilter/Include;../sdk/rapidxml/include;../sdk/libsodium/include;../sdk/opus/include;../sdk/portaudio/include)

target_link_libraries(Gunz zlib.lib;fmodvc.lib;../sdk/libsodium/lib/static/libsodium;legacy_stdio_definitions.lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244 /wd4305")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099 /ignore:4221 /SUBSYSTEM:WINDOWS")

function(link_lib lib_name)
target_link_libraries(Gunz debug ${lib_name}/Debug/${lib_name})
target_link_libraries(Gunz optimized ${lib_name}/Release/${lib_name})
endfunction()

link_lib(cml)
link_lib(RealSpace2)
link_lib(CSCommon)
link_lib(Mint2)
link_lib(RealSound)
link_lib(SafeUDP)
link_lib(MDatabase)

add_subdirectory(../cml ./cml)
add_subdirectory(../RealSpace2 ./RealSpace2)
add_subdirectory(../CSCommon ./CSCommon)
add_subdirectory(../MDatabase ./MDatabase)
add_subdirectory(../Mint2 ./Mint2)
add_subdirectory(../RealSound ./RealSound)
add_subdirectory(../SafeUDP ./SafeUDP)
add_dependencies(RealSpace2 cml)
add_dependencies(CSCommon cml RealSpace2)
add_dependencies(Mint2 cml)
add_dependencies(Gunz cml RealSpace2 MDatabase Mint2 RealSound SafeUDP)