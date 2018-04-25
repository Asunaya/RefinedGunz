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
	../sdk/sqlite/include
	../sdk/dx9/include
	../sdk)

target_link_libraries(Locator PUBLIC
	ini
	cml
	CSCommon
	SafeUDP)
	
if(MSVC)
target_link_libraries(Locator PUBLIC MDatabase)
endif()
