file(GLOB src
    "src/*.h"
    "src/*.cpp"
)

set(main_file "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
list(REMOVE_ITEM src "${main_file}")

add_target(NAME launcher_lib TYPE STATIC SOURCES "${src}")

target_include_directories(launcher_lib PUBLIC
	src)

target_link_libraries(launcher_lib PUBLIC cml curl sodium imgui)

add_target(NAME launcher TYPE EXECUTABLE SOURCES "${main_file}")
target_link_libraries(launcher PUBLIC launcher_lib)

if(MSVC)
	ucm_add_linker_flags(/SUBSYSTEM:WINDOWS)
endif()
