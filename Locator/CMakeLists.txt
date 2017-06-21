file(GLOB src
    "*.h"
    "*.cpp"
)

add_target(NAME Locator TYPE STATIC SOURCES "${src}")

target_include_directories(Locator PRIVATE
	../MatchServer
	../RealSpace2/Include
	../SafeUDP/Include
	../MDatabase/Include
	../MCountryFilter/Include
	../sdk/dx9/include
	../sdk)

find_library(zlib_path NAMES zlib PATHS ../sdk/zlib/lib)
find_library(libsodium_path NAMES libsodium PATHS ../sdk/libsodium/lib/static)
find_library(sqlite_path NAMES sqlite PATHS ../sdk/sqlite/lib/Release)

target_link_libraries(Locator
	legacy_stdio_definitions.lib
	cml
	CSCommon
	SafeUDP
	MDatabase)