link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../sdk/dx9/lib/x86)

if (WIN32)
file(GLOB src
    "./*.h"
    "./*.cpp"
)
else()
set(src main.cpp DB.cpp Launcher.cpp MUtil.cpp SafeString.cpp StringView.cpp)
endif()

add_target(NAME Tests TYPE EXECUTABLE SOURCES "${src}")

target_link_libraries(Tests PUBLIC
	MatchServer_lib
	launcher_lib)

if (WIN32)
find_library(d3dx9_path NAMES d3dx9 PATHS ../sdk/dx9/lib/x86)
find_library(dxerr_path NAMES dxerr PATHS ../sdk/dx9/lib/x86)

target_link_libraries(Tests PUBLIC
	Gunz_lib
	${d3dx9_path}
	${dxerr_path})
endif()
