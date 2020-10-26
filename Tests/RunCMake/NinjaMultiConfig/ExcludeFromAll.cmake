enable_language(C)

set(CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "")
set(CMAKE_DEFAULT_BUILD_TYPE "Release" CACHE STRING "")
set(CMAKE_CROSS_CONFIGS "all" CACHE STRING "")
set(CMAKE_DEFAULT_CONFIGS "all" CACHE STRING "")

add_executable(release_only_tool main.c)
set_property(TARGET release_only_tool PROPERTY EXCLUDE_FROM_ALL "$<NOT:$<CONFIG:Release>>")

include(${CMAKE_CURRENT_LIST_DIR}/Common.cmake)
generate_output_files(release_only_tool)
