# These tests may be configured by cache entries:
#
# CMake_TEST_FindHDF5:BOOL=ON
#   Enable this directory.
#
# CMake_TEST_FindHDF5_<variant>_<lang>_COMPILER:FILEPATH=...
#   Enable testing for a variant/language combination with the given wrapper.
#   Variants: Serial, OpenMPI, MPICH
#   Languages: C, CXX, Fortran
#
# CMake_TEST_FindHDF5_<variant>_<lang>_COMPILER_EXPLICIT:BOOL=ON
#   Pass the above wrapper path to the test as HDF5_<lang>_COMPILER_EXECUTABLE.

set(test_langs C CXX)
if(CMAKE_Fortran_COMPILER)
  list(APPEND test_langs Fortran)
endif()

# Test detection without any special hints.
add_test(NAME FindHDF5.Default COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindHDF5/Test"
  "${CMake_BINARY_DIR}/Tests/FindHDF5/Test-Default"
  ${build_generator_args}
  --build-project TestFindHDF5
  --build-options ${build_options} -DTEST_SERIAL=1 "-DTEST_LANGS=${test_langs}"
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )

foreach(variant Serial OpenMPI MPICH)
  set(wrapper "")
  set(wrapper_langs "")
  set(wrapper_as_compiler "")
  foreach(lang IN LISTS test_langs)
    if(CMake_TEST_FindHDF5_${variant}_${lang}_COMPILER)
      list(APPEND wrapper_langs ${lang})
      list(APPEND wrapper_as_compiler -DCMAKE_${lang}_COMPILER=${CMake_TEST_FindHDF5_${variant}_${lang}_COMPILER})
      if(CMake_TEST_FindHDF5_${variant}_${lang}_COMPILER_EXPLICIT)
        list(APPEND wrapper -DHDF5_${lang}_COMPILER_EXECUTABLE=${CMake_TEST_FindHDF5_${variant}_${lang}_COMPILER})
      endif()
    endif()
  endforeach()

  if(NOT wrapper_langs)
    continue()
  endif()

  if(variant STREQUAL "Serial")
    set(test_kind -DTEST_SERIAL=1)
  else()
    set(test_kind -DTEST_PARALLEL=1)
  endif()

  # Test detection using the HDF5 compiler wrappers as a reference.
  add_test(NAME FindHDF5.${variant} COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindHDF5/Test"
    "${CMake_BINARY_DIR}/Tests/FindHDF5/Test-${variant}"
    ${build_generator_args}
    --build-project TestFindHDF5
    --build-options ${build_options} ${test_kind} "-DTEST_LANGS=${wrapper_langs}" ${wrapper}
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  if(CMake_TEST_FindHDF5_${variant}_ENVMOD)
    set_property(TEST FindHDF5.${variant} PROPERTY ENVIRONMENT_MODIFICATION ${CMake_TEST_FindHDF5_${variant}_ENVMOD})
  endif()

  # Test detection using the HDF5 compiler wrappers as the compiler.
  # Skip this if there are spaces in the path.  The HDF5 wrappers do not like them.
  if(NOT CMAKE_CURRENT_BINARY_DIR MATCHES " ")
    add_test(NAME FindHDF5.${variant}-Wrapper COMMAND
      ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/FindHDF5/Test"
      "${CMake_BINARY_DIR}/Tests/FindHDF5/Test-${variant}-Wrapper"
      ${build_generator_args}
      --build-project TestFindHDF5
      --build-options ${build_options} ${test_kind} "-DTEST_LANGS=${wrapper_langs}" -D TEST_WRAPPER_AS_COMPILER=1 ${wrapper_as_compiler}
      --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
      )
    if(CMake_TEST_FindHDF5_${variant}_ENVMOD)
      set_property(TEST FindHDF5.${variant}-Wrapper PROPERTY ENVIRONMENT_MODIFICATION ${CMake_TEST_FindHDF5_${variant}_ENVMOD})
    endif()
  endif()
endforeach()
