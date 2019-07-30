if(CMake_TEST_FindPython)
  add_test(NAME FindPython.Python2.LOCATION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/Python2"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python2.LOCATION"
    ${build_generator_args}
    --build-project TestPython2
    --build-options ${build_options} -DPython2_FIND_STRATEGY=LOCATION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  add_test(NAME FindPython.Python2.VERSION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/Python2"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python2.VERSION"
    ${build_generator_args}
    --build-project TestPython2
    --build-options ${build_options} -DPython2_FIND_STRATEGY=VERSION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )

  add_test(NAME FindPython.Python2Fail COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/Python2Fail"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python2Fail"
    ${build_generator_args}
    --build-project TestPython2Fail
    --build-options ${build_options}
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  set_tests_properties(FindPython.Python2Fail PROPERTIES
    PASS_REGULAR_EXPRESSION "Could NOT find Python2 \\(missing: foobar\\)")

  add_test(NAME FindPython.Python3.LOCATION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/Python3"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python3.LOCATION"
    ${build_generator_args}
    --build-project TestPython3
    --build-options ${build_options} -DPython3_FIND_STRATEGY=LOCATION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  add_test(NAME FindPython.Python3.VERSION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/Python3"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python3.VERSION"
    ${build_generator_args}
    --build-project TestPython3
    --build-options ${build_options} -DPython3_FIND_STRATEGY=VERSION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )

  add_test(NAME FindPython.Python3Fail COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/Python3Fail"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python3Fail"
    ${build_generator_args}
    --build-project TestPython3Fail
    --build-options ${build_options}
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  set_tests_properties(FindPython.Python3Fail PROPERTIES
    PASS_REGULAR_EXPRESSION "Could NOT find Python3 \\(missing: foobar\\)")

  add_test(NAME FindPython.Python COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/Python"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python"
    ${build_generator_args}
    --build-project TestPython
    --build-options ${build_options}
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )

  add_test(NAME FindPython.MultiplePackages COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/MultiplePackages"
    "${CMake_BINARY_DIR}/Tests/FindPython/MultiplePackages"
    ${build_generator_args}
    --build-project TestMultiplePackages
    --build-options ${build_options}
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )

  add_test(NAME FindPython.VirtualEnv COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/VirtualEnv"
    "${CMake_BINARY_DIR}/Tests/FindPython/VirtualEnv"
    ${build_generator_args}
    --build-project TestVirtualEnv
    --build-options ${build_options}
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )

  add_test(NAME FindPython.Python2Embedded COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/Python2Embedded"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python2Embedded"
    ${build_generator_args}
    --build-project TestPython2Embedded
    --build-options ${build_options}
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  add_test(NAME FindPython.Python3Embedded COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/Python3Embedded"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python3Embedded"
    ${build_generator_args}
    --build-project TestPython3Embedded
    --build-options ${build_options}
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )

endif()

if(CMake_TEST_FindPython_NumPy)
  add_test(NAME FindPython.NumPy COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/NumPy"
    "${CMake_BINARY_DIR}/Tests/FindPython/NumPy"
    ${build_generator_args}
    --build-project TestNumPy
    --build-options ${build_options}
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
  add_test(NAME FindPython.NumPyOnly COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/NumPyOnly"
    "${CMake_BINARY_DIR}/Tests/FindPython/NumPyOnly"
    ${build_generator_args}
    --build-project TestNumPyOnly
    --build-options ${build_options}
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
 endif()
