include(${CMAKE_CURRENT_LIST_DIR}/TestVariable.cmake)

if (PRESET STREQUAL "FileDir")
    test_variable(CMAKE_BINARY_DIR "" "${CMAKE_CURRENT_SOURCE_DIR}/subdir/build")
    test_variable(TEST_FILE_DIR "" "${CMAKE_CURRENT_SOURCE_DIR}/subdir")
elseif (PRESET STREQUAL "FileDirExt")
    test_variable(CMAKE_BINARY_DIR "" "${CMAKE_CURRENT_SOURCE_DIR}/build")
    test_variable(TEST_FILE_DIR "" "${CMAKE_CURRENT_SOURCE_DIR}")
endif()
