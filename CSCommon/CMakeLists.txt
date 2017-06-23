file(GLOB src
    "Include/*.h"
    "Source/*.cpp"
)

add_target(NAME CSCommon TYPE STATIC SOURCES "${src}")

target_include_directories(CSCommon PUBLIC
	.
	Include
	../SafeUDP/Include
	../CML/Include
	../RealSpace2/Include
	../MAI/Include
	../sdk/dx9/Include
	../sdk
	../MDatabase/Include
	../MCountryFilter/Include
	../sdk/rapidxml/include
	../sdk/libsodium/include)
	
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	ucm_add_flags(-D_WINSOCK_DEPRECATED_NO_WARNINGS)
endif()