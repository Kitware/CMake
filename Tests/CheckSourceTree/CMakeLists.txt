add_test(NAME CMake.CheckSourceTree
  COMMAND ${CMAKE_CMAKE_COMMAND} -D GIT_EXECUTABLE=${GIT_EXECUTABLE}
                                 -D CMake_SOURCE_DIR=${CMake_SOURCE_DIR}
                                 -P ${CMAKE_CURRENT_LIST_DIR}/check.cmake
  )
set_property(TEST CMake.CheckSourceTree PROPERTY RUN_SERIAL 1)
