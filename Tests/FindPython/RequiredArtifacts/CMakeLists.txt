cmake_minimum_required(VERSION 3.1)

project(TestRequiredArtifacts LANGUAGES C)

include(CTest)

find_package(Python2 REQUIRED COMPONENTS Interpreter Development)
if (NOT Python2_FOUND)
  message (FATAL_ERROR "Fail to found Python 2")
endif()
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
if (NOT Python3_FOUND)
  message (FATAL_ERROR "Fail to found Python 3")
endif()


add_test(NAME FindPython.RequiredArtifacts.Interpreter.VALID COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindPython/RequiredArtifacts/Check"
  "${CMake_BINARY_DIR}/Tests/FindPython/RequiredArtifacts/Interpreter.VALID"
  ${build_generator_args}
  --build-project TestRequiredArtifacts.Check
  --build-options -DPYTHON_IS_FOUND=TRUE -DCHECK_INTERPRETER=ON
                  "-DPython3_EXECUTABLE=${Python3_EXECUTABLE}"
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
add_test(NAME FindPython.RequiredArtifacts.Interpreter.INVALID COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindPython/RequiredArtifacts/Check"
  "${CMake_BINARY_DIR}/Tests/FindPython/RequiredArtifacts/Interpreter.INVALID"
  ${build_generator_args}
  --build-project TestRequiredArtifacts.Check
  --build-options -DPYTHON_IS_FOUND=FALSE -DCHECK_INTERPRETER=ON
                  "-DPython3_EXECUTABLE=${Python3_EXECUTABLE}-bad${CMAKE_EXECUTABLE_SUFFIX}"
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )

add_test(NAME FindPython.RequiredArtifacts.Library.VALID COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindPython/RequiredArtifacts/Check"
  "${CMake_BINARY_DIR}/Tests/FindPython/RequiredArtifacts/Library.VALID"
  ${build_generator_args}
  --build-project TestRequiredArtifacts.Check
  --build-options -DPYTHON_IS_FOUND=TRUE -DCHECK_LIBRARY=ON
                  "-DPython3_LIBRARY=${Python3_LIBRARY_RELEASE}"
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
add_test(NAME FindPython.RequiredArtifacts.Library.INVALID COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindPython/RequiredArtifacts/Check"
  "${CMake_BINARY_DIR}/Tests/FindPython/RequiredArtifacts/Library.INVALID"
  ${build_generator_args}
  --build-project TestRequiredArtifacts.Check
  --build-options -DPYTHON_IS_FOUND=FALSE -DCHECK_LIBRARY=ON
                  "-DPython3_LIBRARY=${Python2_LIBRARY_RELEASE}"
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )

add_test(NAME FindPython.RequiredArtifacts.Include.VALID COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindPython/RequiredArtifacts/Check"
  "${CMake_BINARY_DIR}/Tests/FindPython/RequiredArtifacts/Include.VALID"
  ${build_generator_args}
  --build-project TestRequiredArtifacts.Check
  --build-options -DPYTHON_IS_FOUND=TRUE -DCHECK_INCLUDE=ON
                  "-DPython3_INCLUDE_DIR=${Python3_INCLUDE_DIRS}"
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
add_test(NAME FindPython.RequiredArtifacts.Include.INVALID COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindPython/RequiredArtifacts/Check"
  "${CMake_BINARY_DIR}/Tests/FindPython/RequiredArtifacts/Include.INVALID"
  ${build_generator_args}
  --build-project TestRequiredArtifacts.Check
  --build-options -DPYTHON_IS_FOUND=FALSE -DCHECK_INCLUDE=ON
                  "-DPython3_INCLUDE_DIR=${Python2_INCLUDE_DIRS}"
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )

add_test(NAME FindPython.RequiredArtifacts.Interpreter-Library.INVALID COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindPython/RequiredArtifacts/Check"
  "${CMake_BINARY_DIR}/Tests/FindPython/RequiredArtifacts/Interpreter-Library.INVALID"
  ${build_generator_args}
  --build-project TestRequiredArtifacts.Check
  --build-options -DPYTHON_IS_FOUND=FALSE -DCHECK_INTERPRETER=ON -DCHECK_LIBRARY=ON
                  "-DPython3_EXECUTABLE=${Python3_EXECUTABLE}"
                  "-DPython3_LIBRARY=${Python2_LIBRARY_RELEASE}"
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )

add_test(NAME FindPython.RequiredArtifacts.Library-Include.INVALID COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindPython/RequiredArtifacts/Check"
  "${CMake_BINARY_DIR}/Tests/FindPython/RequiredArtifacts/Library-Include.INVALID"
  ${build_generator_args}
  --build-project TestRequiredArtifacts.Check
  --build-options -DPYTHON_IS_FOUND=FALSE -DCHECK_LIBRARY=ON -DCHECK_INCLUDE=ON
                  "-DPython3_LIBRARY=${Python3_LIBRARY_RELEASE}"
                  "-DPython3_INCLUDE_DIR=${Python2_INCLUDE_DIRS}"
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
