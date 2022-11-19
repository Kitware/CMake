include(${CMAKE_CURRENT_LIST_DIR}/TestVariable.cmake)

get_filename_component(_parent "${CMAKE_SOURCE_DIR}" DIRECTORY)
file(REAL_PATH "${_parent}" _parent)
test_variable(CMAKE_BINARY_DIR "" "${_parent}/GoodNoSCachePrep-build")
