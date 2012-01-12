configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Test.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/Test.cmake @ONLY)

add_test(CMakeOnly.LinkInterfaceLoop ${CMAKE_CMAKE_COMMAND}
  -DTEST=LinkInterfaceLoop
  -P ${CMAKE_CURRENT_BINARY_DIR}/Test.cmake
  )
set_property(TEST CMakeOnly.LinkInterfaceLoop PROPERTY TIMEOUT 90)
