configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Test.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/Test.cmake @ONLY)

macro(add_CMakeOnly_test test)
  add_test(CMakeOnly.${test} ${CMAKE_CMAKE_COMMAND}
    -DTEST=${test}
    -P ${CMAKE_CURRENT_BINARY_DIR}/Test.cmake
    )
endmacro()

add_CMakeOnly_test(CheckSymbolExists)

add_CMakeOnly_test(CheckCXXSymbolExists)
