cmake_minimum_required(VERSION 2.8)

enable_testing()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(macros)

set(UCM_UNITY_BUILD ON)

add_project_subdir(cml)
add_project_subdir(RealSpace2)
add_project_subdir(CSCommon)
add_project_subdir(Mint2)
add_project_subdir(RealSound)
add_project_subdir(SafeUDP)
add_project_subdir(MDatabase)
add_project_subdir(Gunz)
add_project_subdir(launcher)
add_project_subdir(PatchCreator)
add_project_subdir(Tests)