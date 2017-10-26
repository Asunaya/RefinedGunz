cmake_minimum_required(VERSION 2.8)

enable_testing()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(macros)

if(DEFINED MSSQL)
	set(CMAKE_MFC_FLAG 1)
	add_definitions(-DMSSQL_ENABLED)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	ucm_add_linker_flags(/ignore:4099 /ignore:4221)
	ucm_add_flags(-D_WINSOCK_DEPRECATED_NO_WARNINGS)
endif()

add_project_subdir(sdk/sqlite)
add_project_subdir(sdk/libsodium)
add_project_subdir(sdk/bullet)
add_project_subdir(sdk/rapidxml)

set(UCM_UNITY_BUILD ON)

add_project_subdir(cml)
add_project_subdir(RealSpace2)
add_project_subdir(CSCommon)
add_project_subdir(Mint2)
add_project_subdir(RealSound)
add_project_subdir(SafeUDP)
add_project_subdir(MDatabase)
add_project_subdir(Gunz)
add_project_subdir(Locator)
add_project_subdir(MatchServer)
add_project_subdir(launcher)
add_project_subdir(PatchCreator)
add_project_subdir(PartsIndexer)
add_project_subdir(Tests)