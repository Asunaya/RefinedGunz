cmake_minimum_required(VERSION 2.8)

include(../cmake/macros.cmake)

file(GLOB src
    "Include/*.h"
    "source/*.cpp"
)

add_target(NAME cml TYPE STATIC SOURCES "${src}" PCH_FILE stdafx.h)

target_include_directories(cml PUBLIC .;Include)