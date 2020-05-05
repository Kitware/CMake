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

  add_test(NAME FindPython.Python.LOCATION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/Python"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python.LOCATION"
    ${build_generator_args}
    --build-project TestPython
    --build-options ${build_options} -DPython_FIND_STRATEGY=LOCATION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  add_test(NAME FindPython.Python.VERSION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/Python"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python.VERSION"
    ${build_generator_args}
    --build-project TestPython
    --build-options ${build_options} -DPython_FIND_STRATEGY=VERSION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  add_test(NAME FindPython.Python.V2.LOCATION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/Python"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python.V2.LOCATION"
    ${build_generator_args}
    --build-project TestPython
    --build-options ${build_options} -DPython_REQUESTED_VERSION=2 -DPython_FIND_STRATEGY=LOCATION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  add_test(NAME FindPython.Python.V2.VERSION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/Python"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python.V2.VERSION"
    ${build_generator_args}
    --build-project TestPython
    --build-options ${build_options} -DPython_REQUESTED_VERSION=2 -DPython_FIND_STRATEGY=VERSION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  add_test(NAME FindPython.Python.V3.LOCATION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/Python"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python.V3.LOCATION"
    ${build_generator_args}
    --build-project TestPython
    --build-options ${build_options} -DPython_REQUESTED_VERSION=3 -DPython_FIND_STRATEGY=LOCATION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  add_test(NAME FindPython.Python.V3.VERSION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/Python"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python.V3.VERSION"
    ${build_generator_args}
    --build-project TestPython
    --build-options ${build_options} -DPython_REQUESTED_VERSION=3 -DPython_FIND_STRATEGY=VERSION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )

  add_test(NAME FindPython.Python2.ExactVersion.LOCATION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/ExactVersion"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python2.ExactVersion.LOCATION"
    ${build_generator_args}
    --build-project TestExactVersion
    --build-options ${build_options} -DPython_MAJOR_VERSION=2
                                     -DPython_REQUESTED_VERSION=2.1.2
                                     -DPython2_FIND_STRATEGY=LOCATION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  add_test(NAME FindPython.Python2.ExactVersion.VERSION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/ExactVersion"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python2.ExactVersion.VERSION"
    ${build_generator_args}
    --build-project TestExactVersion
    --build-options ${build_options} -DPython_MAJOR_VERSION=2
                                     -DPython_REQUESTED_VERSION=2.1.2
                                     -DPython2_FIND_STRATEGY=VERSION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  add_test(NAME FindPython.Python3.ExactVersion.LOCATION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/ExactVersion"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python3.ExactVersion.LOCATION"
    ${build_generator_args}
    --build-project TestExactVersion
    --build-options ${build_options} -DPython_MAJOR_VERSION=3
                                     -DPython_REQUESTED_VERSION=3.1.2
                                     -DPython3_FIND_STRATEGY=LOCATION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  add_test(NAME FindPython.Python3.ExactVersion.VERSION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/ExactVersion"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python3.ExactVersion.VERSION"
    ${build_generator_args}
    --build-project TestExactVersion
    --build-options ${build_options} -DPython_MAJOR_VERSION=3
                                     -DPython_REQUESTED_VERSION=3.1.2
                                     -DPython3_FIND_STRATEGY=VERSION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  add_test(NAME FindPython.Python.V2.ExactVersion.LOCATION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/ExactVersion"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python.V2.ExactVersion.LOCATION"
    ${build_generator_args}
    --build-project TestExactVersion
    --build-options ${build_options} -DPython_REQUESTED_VERSION=2.1.2
                                     -DPython_FIND_STRATEGY=LOCATION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  add_test(NAME FindPython.Python.V2.ExactVersion.VERSION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/ExactVersion"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python.V2.ExactVersion.VERSION"
    ${build_generator_args}
    --build-project TestExactVersion
    --build-options ${build_options} -DPython_REQUESTED_VERSION=2.1.2
                                     -DPython_FIND_STRATEGY=VERSION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  add_test(NAME FindPython.Python.V3.ExactVersion.LOCATION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/ExactVersion"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python.V3.ExactVersion.LOCATION"
    ${build_generator_args}
    --build-project TestExactVersion
    --build-options ${build_options} -DPython_REQUESTED_VERSION=3.1.2
                                     -DPython_FIND_STRATEGY=LOCATION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  add_test(NAME FindPython.Python.V3.ExactVersion.VERSION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/ExactVersion"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python.V3.ExactVersion.VERSION"
    ${build_generator_args}
    --build-project TestExactVersion
    --build-options ${build_options} -DPython_REQUESTED_VERSION=3.1.2
                                     -DPython_FIND_STRATEGY=VERSION
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

  add_test(NAME FindPython.RequiredArtifacts COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/RequiredArtifacts"
    "${CMake_BINARY_DIR}/Tests/FindPython/RequiredArtifacts"
    ${build_generator_args}
    --build-project TestRequiredArtifacts
    --build-options ${build_options} "-Dbuild_generator_args=${build_generator_args}"
    "-DCMake_SOURCE_DIR=${CMake_SOURCE_DIR}"
    "-DCMake_BINARY_DIR=${CMake_BINARY_DIR}"
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )

  add_test(NAME FindPython.CustomFailureMessage COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/CustomFailureMessage"
    "${CMake_BINARY_DIR}/Tests/FindPython/CustomFailureMessage"
    ${build_generator_args}
    --build-project TestCustomFailureMessage
    --build-options ${build_options} "-Dbuild_generator_args=${build_generator_args}"
    "-DCMake_SOURCE_DIR=${CMake_SOURCE_DIR}"
    "-DCMake_BINARY_DIR=${CMake_BINARY_DIR}"
    "-DCMake_TEST_FindPython_NumPy=${CMake_TEST_FindPython_NumPy}"
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )

  if (CMAKE_SYSTEM_NAME MATCHES "Linux|Darwin")
    add_test(NAME FindPython.Interpreter.SOABI COMMAND
      ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/FindPython/SOABI"
      "${CMake_BINARY_DIR}/Tests/FindPython/SOABI.Interpreter"
      ${build_generator_args}
      --build-project TestSOABI
      --build-options ${build_options} "-Dbuild_generator_args=${build_generator_args}"
      "-DCMake_SOURCE_DIR=${CMake_SOURCE_DIR}"
      "-DCMake_BINARY_DIR=${CMake_BINARY_DIR}"
      "-DCMake_TEST_FindPython_COMPONENT=Interpreter"
      --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
      )
    add_test(NAME FindPython.Development.SOABI COMMAND
      ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/FindPython/SOABI"
      "${CMake_BINARY_DIR}/Tests/FindPython/SOABI.Development"
      ${build_generator_args}
      --build-project TestSOABI
      --build-options ${build_options} "-Dbuild_generator_args=${build_generator_args}"
      "-DCMake_SOURCE_DIR=${CMake_SOURCE_DIR}"
      "-DCMake_BINARY_DIR=${CMake_BINARY_DIR}"
      "-DCMake_TEST_FindPython_COMPONENT=Development"
      --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
      )
  endif()
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

if(CMake_TEST_FindPython_Conda)
  add_test(NAME FindPython.VirtualEnvConda COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/VirtualEnvConda"
    "${CMake_BINARY_DIR}/Tests/FindPython/VirtualEnvConda"
    ${build_generator_args}
    --build-project TestVirtualEnvConda
    --build-options ${build_options}
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
endif()

if(CMake_TEST_FindPython_IronPython)
  add_test(NAME FindPython.IronPython2.LOCATION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/IronPython2"
    "${CMake_BINARY_DIR}/Tests/FindPython/IronPython2.LOCATION"
    ${build_generator_args}
    --build-project TestIronPython2
    --build-options ${build_options} -DPython2_FIND_STRATEGY=LOCATION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  add_test(NAME FindPython.IronPython2.VERSION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/IronPython2"
    "${CMake_BINARY_DIR}/Tests/FindPython/IronPython2.VERSION"
    ${build_generator_args}
    --build-project TestIronPython2
    --build-options ${build_options} -DPython2_FIND_STRATEGY=VERSION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )

  add_test(NAME FindPython.IronPython.LOCATION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/IronPython"
    "${CMake_BINARY_DIR}/Tests/FindPython/IronPython.LOCATION"
    ${build_generator_args}
    --build-project TestIronPython
    --build-options ${build_options} -DPython_FIND_STRATEGY=LOCATION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  add_test(NAME FindPython.IronPython.VERSION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/IronPython"
    "${CMake_BINARY_DIR}/Tests/FindPython/IronPython.VERSION"
    ${build_generator_args}
    --build-project TestIronPython
    --build-options ${build_options} -DPython_FIND_STRATEGY=VERSION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  add_test(NAME FindPython.IronPython.V2.LOCATION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/IronPython"
    "${CMake_BINARY_DIR}/Tests/FindPython/IronPython.V2.LOCATION"
    ${build_generator_args}
    --build-project TestIronPython
    --build-options ${build_options} -DPython_REQUESTED_VERSION=2 -DPython_FIND_STRATEGY=LOCATION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  add_test(NAME FindPython.IronPython.V2.VERSION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/IronPython"
    "${CMake_BINARY_DIR}/Tests/FindPython/IronPython.V2.VERSION"
    ${build_generator_args}
    --build-project TestIronPython
    --build-options ${build_options} -DPython_REQUESTED_VERSION=2 -DPython_FIND_STRATEGY=VERSION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
endif()
