file(GLOB src
    "*.h"
    "*.cpp"
)

add_target(NAME MatchServer TYPE EXECUTABLE SOURCES "${src}" UNITY_EXCLUDED "MSSQLDatabase.cpp")

target_include_directories(MatchServer PRIVATE
	../sdk/dx9/Include
	../Locator
	../sdk)

target_link_libraries(MatchServer PUBLIC
	zlib
	sodium
	sqlite
	rapidxml
	ini
	cml
	CSCommon
	RealSpace2
	SafeUDP
	Locator)

if (UNIX)
	target_link_libraries(MatchServer PUBLIC dl)
endif()

target_compile_options(MatchServer PRIVATE 
	-D_WINSOCKAPI_)

if (MSVC)
	ucm_add_linker_flags(/SUBSYSTEM:CONSOLE)
	target_link_libraries(MatchServer PUBLIC legacy_stdio_definitions.lib MDatabase)
endif()
