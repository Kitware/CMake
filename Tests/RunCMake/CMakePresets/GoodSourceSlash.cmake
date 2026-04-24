include(${CMAKE_CURRENT_LIST_DIR}/TestVariable.cmake)

get_filename_component(_parent_dir "${CMAKE_SOURCE_DIR}" DIRECTORY)
test_variable(TEST_SOURCE_DIR "PATH" "${CMAKE_SOURCE_DIR}")
test_variable(TEST_SOURCE_PARENT_DIR "PATH" "${_parent_dir}")
test_variable(TEST_SOURCE_DIR_NAME "UNINITIALIZED" "GoodSourceSlash")
