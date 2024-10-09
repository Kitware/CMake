foreach(lang IN ITEMS C CXX Fortran)
  if(CMake_TEST_FindOpenACC_${lang})
    add_test(NAME FindOpenACC.Test${lang} COMMAND
      ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/FindOpenACC/${lang}Test"
      "${CMake_BINARY_DIR}/Tests/FindOpenACC/${lang}Test"
      ${build_generator_args}
      --build-project TestFindOpenACC
      --build-options ${build_options}
      --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
      )
  endif()
endforeach()
