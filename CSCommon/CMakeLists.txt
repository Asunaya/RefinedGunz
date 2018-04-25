file(GLOB src
    "Include/*.h"
    "Source/*.cpp"
)

add_target(NAME CSCommon TYPE STATIC SOURCES "${src}")

target_include_directories(CSCommon PUBLIC
	.
	Include
	../SafeUDP/Include
	../RealSpace2/Include
	../sdk/dx9/Include
	../sdk
	../MDatabase/Include
	../MCountryFilter/Include
	../sdk/rapidxml/include
	../sdk/libsodium/include)

target_link_libraries(CSCommon PUBLIC
	cml
	rapidxml
	sodium)
