include(../cmake/ucm.cmake)

macro(add_target)
ucm_set_runtime(STATIC)
ucm_add_target(${ARGV})
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D_PUBLISH")
endmacro()