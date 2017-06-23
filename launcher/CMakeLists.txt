file(GLOB src
    "src/*.h"
    "src/*.cpp"
)

add_target(NAME launcher TYPE EXECUTABLE SOURCES "${src}")

target_include_directories(launcher PRIVATE
	../cml/include
	../sdk/rapidxml/include
	../sdk/libsodium/include
	../sdk/curl/include
	../src)

add_dependencies(launcher cml)

find_library(curl_path NAMES libcurl PATHS ../sdk/curl/lib/static NO_DEFAULT_PATHS)
add_definitions(-DCURL_STATICLIB)
find_library(libsodium_path NAMES libsodium PATHS ../sdk/libsodium/lib/static NO_DEFAULT_PATHS)
add_definitions(-DSODIUM_STATIC)

target_link_libraries(launcher PRIVATE cml ${curl_path} ${libsodium_path})

set(main_file "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
list(REMOVE_ITEM src "${main_file}")

add_target(NAME launcher_lib TYPE STATIC SOURCES "${src}")

target_include_directories(launcher_lib PRIVATE
	../cml/include
	../sdk/rapidxml/include
	../sdk/libsodium/include
	../sdk/curl/include
	../src)

add_dependencies(launcher_lib cml)

find_library(curl_path NAMES libcurl PATHS ../sdk/curl/lib/static NO_DEFAULT_PATHS)
add_definitions(-DCURL_STATICLIB)
find_library(libsodium_path NAMES libsodium PATHS ../sdk/libsodium/lib/static NO_DEFAULT_PATHS)
add_definitions(-DSODIUM_STATIC)

target_link_libraries(launcher_lib PRIVATE cml ${curl_path} ${libsodium_path})

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	ucm_add_linker_flags(/ignore:4099)
endif()