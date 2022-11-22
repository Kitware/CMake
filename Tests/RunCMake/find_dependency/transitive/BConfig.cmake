message(STATUS "Loading B with components: '${B_FIND_COMPONENTS}'")
include(CMakeFindDependencyMacro)
find_dependency(A NO_DEFAULT_PATH PATHS ${CMAKE_CURRENT_LIST_DIR})
