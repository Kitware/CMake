foreach(c C CXX CUDA Fortran)
  if(CMake_TEST_FindOpenMP_${c})
    set(CMake_TEST_FindOpenMP_FLAG_${c} 1)
  else()
    set(CMake_TEST_FindOpenMP_FLAG_${c} 0)
  endif()
endforeach()

add_test(NAME FindOpenMP.Test COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindOpenMP/Test"
  "${CMake_BINARY_DIR}/Tests/FindOpenMP/Test"
  ${build_generator_args}
  --build-project TestFindOpenMP
  --build-options ${build_options}
  -DOpenMP_TEST_C=${CMake_TEST_FindOpenMP_FLAG_C}
  -DOpenMP_TEST_CXX=${CMake_TEST_FindOpenMP_FLAG_CXX}
  -DOpenMP_TEST_CUDA=${CMake_TEST_FindOpenMP_FLAG_CUDA}
  -DOpenMP_TEST_Fortran=${CMake_TEST_FindOpenMP_FLAG_Fortran}
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
if(CMake_TEST_FindOpenMP_FLAG_CUDA)
  set_property(TEST FindOpenMP.Test APPEND PROPERTY LABELS "CUDA")
endif()
if(CMake_TEST_FindOpenMP_FLAG_Fortran)
  set_property(TEST FindOpenMP.Test APPEND PROPERTY LABELS "Fortran")
endif()

unset(offload_options)
foreach(c C CXX Fortran)
  if(CMake_TEST_FindOpenMP_${c}_OFFLOAD)
    list(APPEND offload_options -DOpenMP_TEST_OFFLOAD_${c}=1)
    if(CMake_TEST_FindOpenMP_${c}_OFFLOAD_FLAGS)
      list(APPEND offload_options "-DOpenMP_${c}_OFFLOAD_FLAGS=${CMake_TEST_FindOpenMP_${c}_OFFLOAD_FLAGS}")
    endif()
  endif()
endforeach()
if(offload_options)
  add_test(NAME FindOpenMP.Offload COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindOpenMP/Offload"
    "${CMake_BINARY_DIR}/Tests/FindOpenMP/Offload"
    ${build_generator_args}
    --build-project TestFindOpenMPOffload
    --build-options ${build_options} ${offload_options}
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  set_property(TEST FindOpenMP.Offload APPEND PROPERTY LABELS "OpenMPOffload")
endif()
