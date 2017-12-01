add_library(zlib INTERFACE)
target_include_directories(zlib INTERFACE include)
find_library(zlib_path NAMES zlib PATHS lib)
target_link_libraries(zlib INTERFACE ${zlib_path})
