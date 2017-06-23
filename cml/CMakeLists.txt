file(GLOB src
    "Include/*.h"
    "source/*.cpp"
    "source/*.c"
)

add_target(NAME cml TYPE STATIC SOURCES "${src}")

target_include_directories(cml PUBLIC
	.
	Include)

find_library(zlib_path NAMES zlib PATHS ../sdk/zlib/lib NO_DEFAULT_PATHS)
target_link_libraries(cml PRIVATE ${zlib_path})