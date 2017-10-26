file(GLOB src
    "./*.h"
    "./*.cpp"
)

add_target(NAME PatchCreator TYPE EXECUTABLE SOURCES "${src}")

target_include_directories(PatchCreator PUBLIC
	../cml/include
	../launcher/src)

target_link_libraries(PatchCreator PRIVATE
	cml
	launcher_lib
	sodium
	rapidxml)