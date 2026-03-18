include(${CMAKE_CURRENT_LIST_DIR}/TestVariable.cmake)

if(NOT "${PRESET}" STREQUAL "FileDirInclude")
    test_variable(CMAKE_BINARY_DIR "" "${CMAKE_CURRENT_SOURCE_DIR}/subdir/build")
    test_variable(TEST_FILE_DIR "" "${CMAKE_CURRENT_SOURCE_DIR}/subdir")
endif()
