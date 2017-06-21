file(GLOB src
    "src/*.h"
    "src/*.cpp"
)

add_target(NAME launcher TYPE EXECUTABLE SOURCES "${src}")

set(main_file "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
list(REMOVE_ITEM src "${main_file}")

add_target(NAME launcher_lib TYPE STATIC SOURCES "${src}")

macro(target_include_directories_both)
	target_include_directories(launcher ${ARGV})
	target_include_directories(launcher_lib ${ARGV})
endmacro()

macro(target_link_libraries_both)
	target_link_libraries(launcher ${ARGV})
	target_link_libraries(launcher_lib ${ARGV})
endmacro()

macro(add_dependencies_both)
	add_dependencies(launcher ${ARGV})
	add_dependencies(launcher_lib ${ARGV})
endmacro()

macro(target_compile_options_both)
	target_compile_options(launcher ${ARGV})
	target_compile_options(launcher_lib ${ARGV})
endmacro()

target_include_directories_both(PUBLIC
	../cml/include
	../sdk/rapidxml/include
	../sdk/libsodium/include
	../sdk/curl/include
	../src)

find_library(curl_path NAMES libcurl PATHS ../sdk/curl/lib/static NO_DEFAULT_PATHS)
add_definitions(-DCURL_STATICLIB)
find_library(libsodium_path NAMES libsodium PATHS ../sdk/libsodium/lib/static NO_DEFAULT_PATHS)
add_definitions(-DSODIUM_STATIC)

target_link_libraries_both(PRIVATE cml ${curl_path} ${libsodium_path})

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	ucm_add_linker_flags(/ignore:4099)
endif()