file(GLOB src
    "*.h"
    "*.cpp"
)

add_target(NAME MatchServer TYPE EXECUTABLE SOURCES "${src}" UNITY_EXCLUDED "MSSQLDatabase.cpp")

target_include_directories(MatchServer PRIVATE
	../sdk/libsodium/include
	../sdk/rapidxml/include
	../sdk/dx9/Include
	../Locator
	../sdk)

find_library(zlib_path NAMES zlib PATHS ../sdk/zlib/lib)
find_library(libsodium_path NAMES libsodium PATHS ../sdk/libsodium/lib/static)
find_library(sqlite_path NAMES sqlite PATHS ../sdk/sqlite/lib/Release)

target_link_libraries(MatchServer PUBLIC
	${zlib_path}
	${libsodium_path}
	${sqlite_path}
	legacy_stdio_definitions.lib
	cml
	CSCommon
	RealSpace2
	SafeUDP
	MDatabase
	Locator)

target_compile_options(MatchServer PRIVATE 
	-DSODIUM_STATIC
	-D_WINSOCKAPI_)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	ucm_add_linker_flags(
		/ignore:4099
		/SUBSYSTEM:CONSOLE)
endif()