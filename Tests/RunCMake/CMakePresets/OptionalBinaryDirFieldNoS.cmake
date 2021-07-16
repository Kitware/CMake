include(${CMAKE_CURRENT_LIST_DIR}/TestVariable.cmake)

get_filename_component(_parent_dir "${CMAKE_SOURCE_DIR}" DIRECTORY)
test_variable(CMAKE_BINARY_DIR "" "${_parent_dir}/OptionalBinaryDirFieldNoS-build")
