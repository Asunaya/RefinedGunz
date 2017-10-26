cmake_policy(SET CMP0015 NEW)

link_directories()
	
file(GLOB src
    "Include/*.h"
    "Source/*.cpp"
)
	
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_USE_GDIPLUS")

add_target(NAME RealSpace2 TYPE STATIC SOURCES "${src}")

target_include_directories(RealSpace2 PUBLIC
	.
	Include
	../CML/Include
	../MAI/Include
	../RealSpace/Include
	../sdk/dx9/include
	../sdk/rapidxml/include
	../sdk)

find_library(dxerr_path NAMES dxerr PATHS ../sdk/dx9/lib/x86)

target_link_libraries(RealSpace2 PUBLIC
	${dxerr_path}
	bullet)