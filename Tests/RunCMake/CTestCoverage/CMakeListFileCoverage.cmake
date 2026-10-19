set(CTEST_TEST_COVERAGE_TOOL "CMAKE")
include(CTest)

# Two tests run the same file, so its counts must be merged.
add_test(NAME Shared1 COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/Shared.cmake)
add_test(NAME Shared2 COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/Shared.cmake)

# A file in the build tree must not be reported.
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/Generated.cmake "set(y 1)\n")
add_test(NAME Generated COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/Generated.cmake)
