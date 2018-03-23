file(GLOB src
    "Include/*.h"
    "source/*.cpp"
    "source/*.c"
)

add_target(NAME cml TYPE STATIC SOURCES "${src}")

target_include_directories(cml PUBLIC
	.
	Include
	../sdk/rapidxml/include)

target_link_libraries(cml PUBLIC zlib)
if (UNIX)
	target_link_libraries(cml PUBLIC pthread)
endif()
