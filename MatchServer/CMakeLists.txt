file(GLOB src
    "*.h"
    "*.cpp"
)

add_target(NAME MatchServer TYPE EXECUTABLE SOURCES "${src}" UNITY_EXCLUDED "MSSQLDatabase.cpp")

target_include_directories(MatchServer PRIVATE
	../sdk/dx9/Include
	../Locator
	../sdk)

find_library(zlib_path NAMES zlib PATHS ../sdk/zlib/lib)

target_link_libraries(MatchServer PUBLIC
	${zlib_path}
	sodium
	sqlite
	rapidxml
	legacy_stdio_definitions.lib
	cml
	CSCommon
	RealSpace2
	SafeUDP
	MDatabase
	Locator)

target_compile_options(MatchServer PRIVATE 
	-D_WINSOCKAPI_)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	ucm_add_linker_flags(/SUBSYSTEM:CONSOLE)
endif()