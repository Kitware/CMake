list(INSERT CMAKE_MODULE_PATH 0
  "${CMAKE_CURRENT_LIST_DIR}/ConfigureLog/cmake")
list(INSERT CMAKE_PREFIX_PATH 0
  "${CMAKE_CURRENT_LIST_DIR}/ConfigureLog")

set(CMAKE_FIND_DEBUG_MODE 1)

# Parameter testing
find_package(ParameterCheck 1.0 EXACT)
find_package(ParameterCheck 1.0...1.5)
find_package(ParameterCheck 1.2)
