cmake_minimum_required(VERSION 2.8)

include(../cmake/macros.cmake)

file(GLOB src
    "Include/*.h"
    "Source/*.cpp"
)

add_target(NAME RealSpace2 TYPE STATIC SOURCES "${src}" PCH_FILE Include/stdafx.h)

target_include_directories(RealSpace2 PUBLIC Include;../CML/Include;../MAI/Include;../RealSpace/Include;../sdk/dx9/include;../sdk/rapidxml/include;../sdk)