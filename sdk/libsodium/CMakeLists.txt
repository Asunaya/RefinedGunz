file(GLOB_RECURSE src
    "src/*.h"
    "src/*.c"
)

add_target(NAME sodium TYPE STATIC SOURCES "${src}")

target_include_directories(sodium
	INTERFACE include
	PRIVATE include/sodium)

target_compile_options(sodium PUBLIC -DSODIUM_STATIC)