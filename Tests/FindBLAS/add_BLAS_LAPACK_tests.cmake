function(add_BLAS_LAPACK_tests var)
  if(var MATCHES "^CMake_TEST_Find(BLAS|LAPACK)$")
    set(package "${CMAKE_MATCH_1}")
  else()
    message(FATAL_ERROR "Test list variable '${var}' not supported.")
  endif()

  set(all "")
  set(compiler "")
  set(static "")

  foreach(variant IN LISTS ${var})
    if(variant MATCHES "^(all|compiler|static)=(.*)$")
      set("${CMAKE_MATCH_1}" "${CMAKE_MATCH_2}")
      continue()
    elseif(variant MATCHES "^([^=]+)=(.*)$")
      message(FATAL_ERROR "Unknown variable '${variant}'")
    endif()
    set(variant_name "${variant}")
    set(variant_options "-DBLA_VENDOR=${variant}")
    if(variant STREQUAL "All" AND all)
      list(APPEND variant_options "-DEXPECT_All=${all}")
    endif()
    if(compiler)
      string(APPEND variant_name "_${compiler}")
      list(APPEND variant_options "-DCMAKE_C_COMPILER=${compiler}")
    endif()
    if(static)
      string(APPEND variant_name "_Static")
      list(APPEND variant_options "-DBLA_STATIC=ON")
    endif()
    add_test(NAME Find${package}.Test_${variant_name} COMMAND
      ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/Find${package}/Test"
      "${CMake_BINARY_DIR}/Tests/Find${package}/Test_${variant_name}"
      ${build_generator_args}
      --build-project TestFind${package}
      --build-options ${build_options} ${variant_options}
      --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
      )
  endforeach()
endfunction()
