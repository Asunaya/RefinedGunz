cmake_minimum_required(VERSION 2.8)

include(../cmake/macros.cmake)

file(GLOB src
    "Include/*.h"
    "Source/*.cpp"
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNO_WARN_MBCS_MFC_DEPRECATION")

add_target(NAME MDatabase TYPE STATIC SOURCES "${src}" PCH_FILE stdafx.h)

target_include_directories(MDatabase PUBLIC .;include;../cml/Include/)
