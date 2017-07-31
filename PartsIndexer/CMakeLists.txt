file(GLOB src
    "PartsIndexer/*.h"
    "PartsIndexer/*.cpp"
)

add_target(NAME PartsIndexer TYPE EXECUTABLE SOURCES "${src}")