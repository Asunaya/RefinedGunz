file(GLOB src
    "src/*.h"
    "src/*.cpp"
)

set(main_file "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
list(REMOVE_ITEM src "${main_file}")
set(imgui_impl_dx9 "${CMAKE_CURRENT_SOURCE_DIR}/src/imgui_impl_dx9.cpp")
list(REMOVE_ITEM src "${imgui_impl_dx9}")

add_target(NAME launcher_lib TYPE STATIC SOURCES "${src}")

target_include_directories(launcher_lib PUBLIC
	src)

target_link_libraries(launcher_lib PUBLIC cml curl sodium)

if (WIN32)
	add_target(NAME launcher TYPE EXECUTABLE SOURCES "${main_file}" "${imgui_impl_dx9}")
	target_link_libraries(launcher PUBLIC launcher_lib imgui)

	if(MSVC)
		ucm_add_linker_flags(/SUBSYSTEM:WINDOWS)
	endif()
endif()
