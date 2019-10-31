cmake_minimum_required(VERSION 3.1)

project(TestCustomFailureMessage LANGUAGES NONE)

include(CTest)

add_test(NAME FindPython.CustomFailureMessage.Interpreter COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindPython/CustomFailureMessage/Check"
  "${CMake_BINARY_DIR}/Tests/FindPython/CustomFailureMessage/Interpreter"
  ${build_generator_args}
  --build-project TestCustomFailureMessage.Check
  --build-options "-DCHECK_COMPONENTS=Interpreter"
                  "-DPython3_EXECUTABLE=/not/found/interpreter"
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
set_tests_properties(FindPython.CustomFailureMessage.Interpreter PROPERTIES
  PASS_REGULAR_EXPRESSION "Reason given by package:.+Interpreter: Cannot run the interpreter")

add_test(NAME FindPython.CustomFailureMessage.Library COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindPython/CustomFailureMessage/Check"
  "${CMake_BINARY_DIR}/Tests/FindPython/CustomFailureMessage/Library"
  ${build_generator_args}
  --build-project TestCustomFailureMessage.Check
  --build-options "-DCHECK_COMPONENTS=Development"
                  "-DPython3_LIBRARY=/not/found/library"
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
set_tests_properties(FindPython.CustomFailureMessage.Library PROPERTIES
  PASS_REGULAR_EXPRESSION "Reason given by package:.+Development: Cannot find the library")

add_test(NAME FindPython.CustomFailureMessage.Include COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindPython/CustomFailureMessage/Check"
  "${CMake_BINARY_DIR}/Tests/FindPython/CustomFailureMessage/Include"
  ${build_generator_args}
  --build-project TestCustomFailureMessage.Check
  --build-options "-DCHECK_COMPONENTS=Development"
                  "-DPython3_INCLUDE_DIR=/not/found/include"
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
set_tests_properties(FindPython.CustomFailureMessage.Include PROPERTIES
  PASS_REGULAR_EXPRESSION "Reason given by package:.+Development: Cannot find the directory")

add_test(NAME FindPython.CustomFailureMessage.Multiple COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindPython/CustomFailureMessage/Check"
  "${CMake_BINARY_DIR}/Tests/FindPython/CustomFailureMessage/Multiple"
  ${build_generator_args}
  --build-project TestCustomFailureMessage.Check
  --build-options "-DCHECK_COMPONENTS=Interpreter;Development"
                  "-DPython3_EXECUTABLE=/not/found/interpreter"
                  "-DPython3_LIBRARY=/not/found/library"
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
set_tests_properties(FindPython.CustomFailureMessage.Multiple PROPERTIES
  PASS_REGULAR_EXPRESSION "Reason given by package:.+Interpreter: Cannot run the interpreter.+Development: Cannot find the library")


if (CMake_TEST_FindPython_NumPy)
  add_test(NAME FindPython.CustomFailureMessage.NumPy COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/CustomFailureMessage/Check"
    "${CMake_BINARY_DIR}/Tests/FindPython/CustomFailureMessage/NumPy"
    ${build_generator_args}
    --build-project TestCustomFailureMessage.Check
    --build-options "-DCHECK_COMPONENTS=Interpreter;Development;NumPy"
                    "-DPython3_NumPy_INCLUDE_DIR=/not/found/numpy/include"
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  set_tests_properties(FindPython.CustomFailureMessage.NumPy PROPERTIES
    PASS_REGULAR_EXPRESSION "Reason given by package:.+NumPy: Cannot find the directory")
endif()
