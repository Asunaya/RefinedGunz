cmake_minimum_required(VERSION 2.8)

include(../cmake/macros.cmake)

file(GLOB src
    "Include/*.h"
    "Source/*.cpp"
)

add_target(NAME CSCommon TYPE STATIC SOURCES "${src}" PCH_FILE stdafx.h)

target_include_directories(CSCommon PUBLIC Include;../SafeUDP/Include;../CML/Include;../RealSpace2/Include;../MAI/Include;../sdk/dx9/Include;../sdk;../MDatabase/Include;../MCountryFilter/Include;../sdk/rapidxml/include;../sdk/libsodium/include)