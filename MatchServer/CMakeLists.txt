file(GLOB src
    "*.h"
    "*.cpp"
)

set(main_file "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
list(REMOVE_ITEM src "${main_file}")

add_target(NAME MatchServer_lib TYPE STATIC SOURCES "${src}")

target_include_directories(MatchServer_lib PUBLIC
	.
	../sdk/dx9/Include
	../Locator
	../sdk)

target_link_libraries(MatchServer_lib PUBLIC
	zlib
	sodium
	sqlite
	rapidxml
	ini
	cml
	CSCommon
	MDatabase
	RealSpace2
	SafeUDP
	Locator)

if (UNIX)
	target_link_libraries(MatchServer_lib PUBLIC dl)
endif()

if (MSVC)
	ucm_add_linker_flags(/SUBSYSTEM:CONSOLE)
	target_link_libraries(MatchServer_lib PUBLIC legacy_stdio_definitions.lib)
endif()

add_target(NAME MatchServer TYPE EXECUTABLE SOURCES "${main_file}")
target_link_libraries(MatchServer PUBLIC MatchServer_lib)