link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../sdk/dx9/lib/x86)

file(GLOB src
    "./*.h"
    "./*.cpp"
)

add_target(NAME Tests TYPE EXECUTABLE SOURCES "${src}")

find_library(d3dx9_path NAMES d3dx9 PATHS ../sdk/dx9/lib/x86)
find_library(dxerr_path NAMES dxerr PATHS ../sdk/dx9/lib/x86)

target_link_libraries(Tests PRIVATE
	Gunz_lib
	launcher_lib
	${d3dx9_path}
	${dxerr_path}
	legacy_stdio_definitions.lib)
