cmake_minimum_required(VERSION 2.8)

include(../cmake/macros.cmake)

file(GLOB src
    "Include/*.h"
    "source/*.cpp"
)

add_target(NAME SafeUDP TYPE STATIC SOURCES "${src}" PCH_FILE Include/stdafx.h)

target_include_directories(SafeUDP PUBLIC Include;../CML/Include)