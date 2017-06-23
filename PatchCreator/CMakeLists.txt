file(GLOB src
    "./*.h"
    "./*.cpp"
)

add_target(NAME PatchCreator TYPE EXECUTABLE SOURCES "${src}")

target_include_directories(PatchCreator PUBLIC
	../cml/include
	../sdk/rapidxml/include
	../sdk/libsodium/include
	../launcher/src)

add_dependencies(PatchCreator cml)
add_dependencies(PatchCreator launcher_lib)

find_library(libsodium_path NAMES libsodium PATHS ../sdk/libsodium/lib/static)
add_definitions(-DSODIUM_STATIC)

target_link_libraries(PatchCreator PRIVATE cml launcher_lib ${libsodium_path})

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	ucm_add_linker_flags(/ignore:4099)
endif()