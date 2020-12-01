enable_language(C)

set(CMAKE_CONFIGURATION_TYPES "Release;Debug" CACHE STRING "")

add_executable(release_only_tool main.c)
set_property(TARGET release_only_tool PROPERTY EXCLUDE_FROM_ALL "$<NOT:$<CONFIG:Release>>")
