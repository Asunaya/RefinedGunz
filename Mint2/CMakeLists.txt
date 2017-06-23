file(GLOB src
    "Include/*.h"
    "Source/*.cpp"
)

add_target(NAME Mint2 TYPE STATIC SOURCES "${src}")

target_include_directories(Mint2 PUBLIC
	.
	Include
	../CML/Include)
