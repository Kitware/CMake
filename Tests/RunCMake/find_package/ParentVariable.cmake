message(STATUS "ParentVariable\.cmake: '${CMAKE_PARENT_LIST_FILE}'")
set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/ParentVariable)
find_package(Primary QUIET CONFIG REQUIRED)
message(STATUS "ParentVariable\.cmake: '${CMAKE_PARENT_LIST_FILE}'")
