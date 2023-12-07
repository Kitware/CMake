if(CMake_TEST_FindPython2)
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

  add_test(NAME FindPython.Python2.Development.Module COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/Python2Module"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python2Module"
    ${build_generator_args}
    --build-project TestPython2Module
    --build-options ${build_options}
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

  add_test(NAME FindPython.Python2.VersionRange.LOCATION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/VersionRange"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python2.VersionRange.LOCATION"
    ${build_generator_args}
    --build-project TestVersionRange
    --build-options ${build_options} -DPython=Python2 -DPython_REQUESTED_VERSION=2
                                     -DPython2_FIND_STRATEGY=LOCATION
    )
  add_test(NAME FindPython.Python2.VersionRange.VERSION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/VersionRange"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python2.VersionRange.VERSION"
    ${build_generator_args}
    --build-project TestVersionRange
    --build-options ${build_options} -DPython=Python2 -DPython_REQUESTED_VERSION=2
                                     -DPython2_FIND_STRATEGY=VERSION
    )
  add_test(NAME FindPython.Python.V2.VersionRange.LOCATION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/VersionRange"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python.V2.VersionRange.LOCATION"
    ${build_generator_args}
    --build-project TestVersionRange
    --build-options ${build_options} -DPython=Python -DPython_REQUESTED_VERSION=2
                                     -DPython_FIND_STRATEGY=LOCATION
    )
  add_test(NAME FindPython.Python.V2.VersionRange.VERSION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/VersionRange"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python.V2.VersionRange.VERSION"
    ${build_generator_args}
    --build-project TestVersionRange
    --build-options ${build_options} -DPython=Python -DPython_REQUESTED_VERSION=2
                                     -DPython_FIND_STRATEGY=VERSION
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

  set_property(TEST FindPython.Python2.LOCATION FindPython.Python2.VERSION
                    FindPython.Python2.Development.Module FindPython.Python2Fail
                    FindPython.Python.V2.LOCATION FindPython.Python.V2.VERSION
                    FindPython.Python2.ExactVersion.LOCATION FindPython.Python2.ExactVersion.VERSION
                    FindPython.Python.V2.ExactVersion.LOCATION FindPython.Python.V2.ExactVersion.VERSION
                    FindPython.Python2.VersionRange.LOCATION FindPython.Python2.VersionRange.VERSION
                    FindPython.Python.V2.VersionRange.LOCATION FindPython.Python.V2.VersionRange.VERSION
                    FindPython.Python2Embedded
               APPEND PROPERTY LABELS Python2)
endif()

if(CMake_TEST_FindPython3)
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

  add_test(NAME FindPython.Python3.Development.Module COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/Python3Module"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python3Module"
    ${build_generator_args}
    --build-project TestPython3Module
    --build-options ${build_options}
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

  add_test(NAME FindPython.Python3.VersionRange.LOCATION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/VersionRange"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python3.VersionRange.LOCATION"
    ${build_generator_args}
    --build-project TestVersionRange
    --build-options ${build_options} -DPython=Python3 -DPython_REQUESTED_VERSION=3
                                     -DPython3_FIND_STRATEGY=LOCATION
    )
  add_test(NAME FindPython.Python3.VersionRange.VERSION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/VersionRange"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python3.VersionRange.VERSION"
    ${build_generator_args}
    --build-project TestVersionRange
    --build-options ${build_options} -DPython=Python3 -DPython_REQUESTED_VERSION=3
                                     -DPython3_FIND_STRATEGY=VERSION
    )
  add_test(NAME FindPython.Python.V3.VersionRange.LOCATION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/VersionRange"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python.V3.VersionRange.LOCATION"
    ${build_generator_args}
    --build-project TestVersionRange
    --build-options ${build_options} -DPython=Python -DPython_REQUESTED_VERSION=3
                                     -DPython_FIND_STRATEGY=LOCATION
    )
  add_test(NAME FindPython.Python.V3.VersionRange.VERSION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/VersionRange"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python.V3.VersionRange.VERSION"
    ${build_generator_args}
    --build-project TestVersionRange
    --build-options ${build_options} -DPython=Python -DPython_REQUESTED_VERSION=3
                                     -DPython_FIND_STRATEGY=VERSION
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
    "-DCMake_TEST_FindPython3_SABIModule=${CMake_TEST_FindPython3_SABIModule}"
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )

  add_test(NAME FindPython.ArtifactsInteractive.ON COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/ArtifactsInteractive"
    "${CMake_BINARY_DIR}/Tests/FindPython/ArtifactsInteractive.ON"
    ${build_generator_args}
    --build-project TestArtifactsScope
    --build-options ${build_options} "-Dbuild_generator_args=${build_generator_args}"
    "-DCMake_SOURCE_DIR=${CMake_SOURCE_DIR}"
    "-DCMake_BINARY_DIR=${CMake_BINARY_DIR}"
    "-DCMake_TEST_FindPython3_NumPy=${CMake_TEST_FindPython3_NumPy}"
    "-DPython3_ARTIFACTS_INTERACTIVE=ON"
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  add_test(NAME FindPython.ArtifactsInteractive.OFF COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/ArtifactsInteractive"
    "${CMake_BINARY_DIR}/Tests/FindPython/ArtifactsInteractive.OFF"
    ${build_generator_args}
    --build-project TestArtifactsScope
    --build-options ${build_options} "-Dbuild_generator_args=${build_generator_args}"
    "-DCMake_SOURCE_DIR=${CMake_SOURCE_DIR}"
    "-DCMake_BINARY_DIR=${CMake_BINARY_DIR}"
    "-DCMake_TEST_FindPython3_NumPy=${CMake_TEST_FindPython3_NumPy}"
    "-DPython3_ARTIFACTS_INTERACTIVE=OFF"
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
    "-DCMake_TEST_FindPython3_NumPy=${CMake_TEST_FindPython3_NumPy}"
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )

  add_test(NAME FindPython.DifferentComponents COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/DifferentComponents"
    "${CMake_BINARY_DIR}/Tests/FindPython/DifferentComponents"
    ${build_generator_args}
    --build-project DifferentComponents
    --build-options ${build_options} "-Dbuild_generator_args=${build_generator_args}"
    "-DCMake_SOURCE_DIR=${CMake_SOURCE_DIR}"
    "-DCMake_BINARY_DIR=${CMake_BINARY_DIR}"
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )

  set_property(TEST FindPython.Python3.LOCATION FindPython.Python3.VERSION
                    FindPython.Python3.Development.Module FindPython.Python3Fail
                    FindPython.Python.V3.LOCATION FindPython.Python.V3.VERSION
                    FindPython.Python3.ExactVersion.LOCATION FindPython.Python3.ExactVersion.VERSION
                    FindPython.Python.V3.ExactVersion.LOCATION FindPython.Python.V3.ExactVersion.VERSION
                    FindPython.Python3.VersionRange.LOCATION FindPython.Python3.VersionRange.VERSION
                    FindPython.Python.V3.VersionRange.LOCATION FindPython.Python.V3.VersionRange.VERSION
                    FindPython.VirtualEnv FindPython.Python3Embedded FindPython.RequiredArtifacts
                    FindPython.ArtifactsInteractive.ON FindPython.ArtifactsInteractive.OFF
                    FindPython.CustomFailureMessage FindPython.DifferentComponents
               APPEND PROPERTY LABELS Python3)

  if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_test(NAME FindPython.UnversionedNames COMMAND
      ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/FindPython/UnversionedNames"
      "${CMake_BINARY_DIR}/Tests/FindPython/UnversionedNames"
      ${build_generator_args}
      --build-project UnversionedNames
      --build-options ${build_options}
    )
    set_property(TEST FindPython.UnversionedNames APPEND PROPERTY LABELS Python3)
  endif()
endif()

if(CMake_TEST_FindPython2 OR CMake_TEST_FindPython3)
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

    set_property(TEST FindPython.Interpreter.SOABI FindPython.Development.SOABI
                 APPEND PROPERTY LABELS Python2 Python3)
  endif()

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

  set_property(TEST FindPython.Python.LOCATION FindPython.Python.VERSION FindPython.MultiplePackages
               APPEND PROPERTY LABELS Python2 Python3)
endif()


if(CMake_TEST_FindPython2_SABIModule)
  add_test(NAME FindPython.Python2.Development.SABIModule COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/Python2SABIModule"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python2SABIModule"
    ${build_generator_args}
    --build-project TestPython2SABIModule
    --build-options ${build_options}
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  set_tests_properties(FindPython.Python2.Development.SABIModule PROPERTIES
    PASS_REGULAR_EXPRESSION "Could NOT find Python2 \\(missing: .*Development\\.SABIModule")
  set_property(TEST FindPython.Python2.Development.SABIModule APPEND PROPERTY LABELS Python2)
endif()

if(CMake_TEST_FindPython3_SABIModule)
  # Use exclusively Release configuration because Debug is, on Windows with MSVC,
  # unusable with SABI: Python force link with debug version of full versioned library rather than
  # the stable ABI one.
  add_test(NAME FindPython.Python3.Development.SABIModule COMMAND
    ${CMAKE_CTEST_COMMAND} -C Release
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/Python3SABIModule"
    "${CMake_BINARY_DIR}/Tests/FindPython/Python3SABIModule"
    ${build_generator_args}
    --build-project TestPython3SABIModule
    --build-options ${build_options}
    --test-command ${CMAKE_CTEST_COMMAND} -V -C Release
    )
  set_property(TEST FindPython.Python3.Development.SABIModule APPEND PROPERTY LABELS Python3)
endif()

if(CMake_TEST_FindPython2_NumPy OR CMake_TEST_FindPython3_NumPy)
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
  )

  set_property(TEST FindPython.NumPy FindPython.NumPyOnly APPEND PROPERTY LABELS Python2 Python3)
endif()

if(CMake_TEST_FindPython3_Conda)
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
  set_property(TEST FindPython.VirtualEnvConda APPEND PROPERTY LABELS Python3)
endif()

if (CMake_TEST_FindPython2 AND CMake_TEST_FindPython2_IronPython)
  add_test(NAME FindPython.Implementation.CPython2 COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/Implementation"
    "${CMake_BINARY_DIR}/Tests/FindPython/Implementation.CPython2"
    ${build_generator_args}
    --build-project TestImplementationCPython
    --build-options ${build_options} -DPython_REQUESTED_VERSION=2 -DPython_REQUESTED_IMPLEMENTATIONS=CPython
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  add_test(NAME FindPython.Implementation.IronPython2 COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/Implementation"
    "${CMake_BINARY_DIR}/Tests/FindPython/Implementation.IronPython2"
    ${build_generator_args}
    --build-project TestImplementationIronPython
    --build-options ${build_options} -DPython_REQUESTED_VERSION=2 -DPython_REQUESTED_IMPLEMENTATION=IronPython
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )

  set_property(TEST FindPython.Implementation.CPython2 FindPython.Implementation.IronPython2
               APPEND PROPERTY LABELS Python2)
endif()

if (CMake_TEST_FindPython3 AND CMake_TEST_FindPython3_IronPython)
  add_test(NAME FindPython.Implementation.CPython3 COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/Implementation"
    "${CMake_BINARY_DIR}/Tests/FindPython/Implementation.CPython3"
    ${build_generator_args}
    --build-project TestImplementationCPython
    --build-options ${build_options} -DPython_REQUESTED_VERSION=3 -DPython_REQUESTED_IMPLEMENTATIONS=CPython
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  add_test(NAME FindPython.Implementation.IronPython3 COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/Implementation"
    "${CMake_BINARY_DIR}/Tests/FindPython/Implementation.IronPython3"
    ${build_generator_args}
    --build-project TestImplementationIronPython
    --build-options ${build_options} -DPython_REQUESTED_VERSION=3 -DPython_REQUESTED_IMPLEMENTATION=IronPython
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )

  set_property(TEST FindPython.Implementation.CPython3 FindPython.Implementation.IronPython3
               APPEND PROPERTY LABELS Python3)
endif()

if(CMake_TEST_FindPython2_IronPython)
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

  add_test(NAME FindPython.IronPython2.VersionRange.LOCATION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/VersionRange"
    "${CMake_BINARY_DIR}/Tests/FindPython/IronPython2.VersionRange.LOCATION"
    ${build_generator_args}
    --build-project TestVersionRange
    --build-options ${build_options} -DPython=Python2 -DPython_REQUESTED_VERSION=2
                                     -DPython2_FIND_IMPLEMENTATIONS=IronPython
                                     -DPython2_FIND_STRATEGY=LOCATION
    )
  add_test(NAME FindPython.IronPython2.VersionRange.VERSION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/VersionRange"
    "${CMake_BINARY_DIR}/Tests/FindPython/IronPython2.VersionRange.VERSION"
    ${build_generator_args}
    --build-project TestVersionRange
    --build-options ${build_options} -DPython=Python2 -DPython_REQUESTED_VERSION=2
                                     -DPython2_FIND_IMPLEMENTATIONS=IronPython
                                     -DPython2_FIND_STRATEGY=VERSION
    )

  set_property(TEST FindPython.IronPython2.LOCATION FindPython.IronPython2.VERSION
                    FindPython.IronPython.V2.LOCATION FindPython.IronPython.V2.VERSION
                    FindPython.IronPython2.VersionRange.LOCATION FindPython.IronPython2.VersionRange.VERSION
               APPEND PROPERTY LABELS Python2)
endif()

if(CMake_TEST_FindPython3_IronPython)
  add_test(NAME FindPython.IronPython3.LOCATION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/IronPython3"
    "${CMake_BINARY_DIR}/Tests/FindPython/IronPython3.LOCATION"
    ${build_generator_args}
    --build-project TestIronPython3
    --build-options ${build_options} -DPython3_FIND_STRATEGY=LOCATION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  add_test(NAME FindPython.IronPython3.VERSION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/IronPython3"
    "${CMake_BINARY_DIR}/Tests/FindPython/IronPython3.VERSION"
    ${build_generator_args}
    --build-project TestIronPython3
    --build-options ${build_options} -DPython3_FIND_STRATEGY=VERSION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )

  add_test(NAME FindPython.IronPython.V3.LOCATION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/IronPython"
    "${CMake_BINARY_DIR}/Tests/FindPython/IronPython.V3.LOCATION"
    ${build_generator_args}
    --build-project TestIronPython
    --build-options ${build_options} -DPython_REQUESTED_VERSION=3 -DPython_FIND_STRATEGY=LOCATION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  add_test(NAME FindPython.IronPython.V3.VERSION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/IronPython"
    "${CMake_BINARY_DIR}/Tests/FindPython/IronPython.V3.VERSION"
    ${build_generator_args}
    --build-project TestIronPython
    --build-options ${build_options} -DPython_REQUESTED_VERSION=3 -DPython_FIND_STRATEGY=VERSION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )

  add_test(NAME FindPython.IronPython3.VersionRange.LOCATION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/VersionRange"
    "${CMake_BINARY_DIR}/Tests/FindPython/IronPython3.VersionRange.LOCATION"
    ${build_generator_args}
    --build-project TestVersionRange
    --build-options ${build_options} -DPython=Python3 -DPython_REQUESTED_VERSION=3
                                     -DPython3_FIND_IMPLEMENTATIONS=IronPython
                                     -DPython3_FIND_STRATEGY=LOCATION
    )
  add_test(NAME FindPython.IronPython3.VersionRange.VERSION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/VersionRange"
    "${CMake_BINARY_DIR}/Tests/FindPython/IronPython3.VersionRange.VERSION"
    ${build_generator_args}
    --build-project TestVersionRange
    --build-options ${build_options} -DPython=Python3 -DPython_REQUESTED_VERSION=3
                                     -DPython3_FIND_IMPLEMENTATIONS=IronPython
                                     -DPython3_FIND_STRATEGY=VERSION
    )

  set_property(TEST FindPython.IronPython3.LOCATION FindPython.IronPython3.VERSION
                    FindPython.IronPython.V3.LOCATION FindPython.IronPython.V3.VERSION
                    FindPython.IronPython3.VersionRange.LOCATION FindPython.IronPython3.VersionRange.VERSION
               APPEND PROPERTY LABELS Python3)
endif()

if(CMake_TEST_FindPython2_IronPython OR CMake_TEST_FindPython3_IronPython)
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

  set_property(TEST FindPython.IronPython.LOCATION FindPython.IronPython.VERSION
               APPEND PROPERTY LABELS Python2 Python3)
endif()

if(CMake_TEST_FindPython2_PyPy)
  add_test(NAME FindPython.PyPy2.LOCATION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/PyPy2"
    "${CMake_BINARY_DIR}/Tests/FindPython/PyPy2.LOCATION"
    ${build_generator_args}
    --build-project TestPyPy2
    --build-options ${build_options} -DPython2_FIND_STRATEGY=LOCATION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  add_test(NAME FindPython.PyPy2.VERSION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/PyPy2"
    "${CMake_BINARY_DIR}/Tests/FindPython/PyPy2.VERSION"
    ${build_generator_args}
    --build-project TestPyPy2
    --build-options ${build_options} -DPython2_FIND_STRATEGY=VERSION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )

  add_test(NAME FindPython.PyPy.V2.LOCATION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/PyPy"
    "${CMake_BINARY_DIR}/Tests/FindPython/PyPy.V2.LOCATION"
    ${build_generator_args}
    --build-project TestPyPy
    --build-options ${build_options} -DPython_REQUESTED_VERSION=2 -DPython_FIND_STRATEGY=LOCATION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  add_test(NAME FindPython.PyPy.V2.VERSION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/PyPy"
    "${CMake_BINARY_DIR}/Tests/FindPython/PyPy.V2.VERSION"
    ${build_generator_args}
    --build-project TestPyPy
    --build-options ${build_options} -DPython_REQUESTED_VERSION=2 -DPython_FIND_STRATEGY=VERSION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )

  set_property(TEST FindPython.PyPy2.LOCATION FindPython.PyPy2.VERSION
                    FindPython.PyPy.V2.LOCATION FindPython.PyPy.V2.VERSION
               APPEND PROPERTY LABELS Python2)
endif()

if(CMake_TEST_FindPython3_PyPy)
  add_test(NAME FindPython.PyPy3.LOCATION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/PyPy3"
    "${CMake_BINARY_DIR}/Tests/FindPython/PyPy3.LOCATION"
    ${build_generator_args}
    --build-project TestPyPy3
    --build-options ${build_options} -DPython3_FIND_STRATEGY=LOCATION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  add_test(NAME FindPython.PyPy3.VERSION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/PyPy3"
    "${CMake_BINARY_DIR}/Tests/FindPython/PyPy3.VERSION"
    ${build_generator_args}
    --build-project TestPyPy3
    --build-options ${build_options} -DPython3_FIND_STRATEGY=VERSION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )

  add_test(NAME FindPython.PyPy.V3.LOCATION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/PyPy"
    "${CMake_BINARY_DIR}/Tests/FindPython/PyPy.V3.LOCATION"
    ${build_generator_args}
    --build-project TestPyPy
    --build-options ${build_options} -DPython_REQUESTED_VERSION=3 -DPython_FIND_STRATEGY=LOCATION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  add_test(NAME FindPython.PyPy.V3.VERSION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/PyPy"
    "${CMake_BINARY_DIR}/Tests/FindPython/PyPy.V3.VERSION"
    ${build_generator_args}
    --build-project TestPyPy
    --build-options ${build_options} -DPython_REQUESTED_VERSION=3 -DPython_FIND_STRATEGY=VERSION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )

  set_property(TEST FindPython.PyPy3.LOCATION FindPython.PyPy3.VERSION
                    FindPython.PyPy.V3.LOCATION FindPython.PyPy.V3.VERSION
               APPEND PROPERTY LABELS Python3)
endif()

if(CMake_TEST_FindPython2_PyPy OR CMake_TEST_FindPython3_PyPy)
  add_test(NAME FindPython.PyPy.LOCATION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/PyPy"
    "${CMake_BINARY_DIR}/Tests/FindPython/PyPy.LOCATION"
    ${build_generator_args}
    --build-project TestPyPy
    --build-options ${build_options} -DPython_FIND_STRATEGY=LOCATION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  add_test(NAME FindPython.PyPy.VERSION COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/PyPy"
    "${CMake_BINARY_DIR}/Tests/FindPython/PyPy.VERSION"
    ${build_generator_args}
    --build-project TestPyPy
    --build-options ${build_options} -DPython_FIND_STRATEGY=VERSION
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )

  set_property(TEST FindPython.PyPy.LOCATION FindPython.PyPy.VERSION
               APPEND PROPERTY LABELS Python2 Python3)
endif()
