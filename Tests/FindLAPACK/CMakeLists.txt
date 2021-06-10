foreach(variant IN LISTS CMake_TEST_FindLAPACK)
  if(variant MATCHES "^([^.]+)\\.(.*)$")
    set(vendor "${CMAKE_MATCH_1}")
    set(alt_compiler "-DCMAKE_C_COMPILER=${CMAKE_MATCH_2}")
  else()
    set(vendor "${variant}")
    set(alt_compiler "")
  endif()
  add_test(NAME FindLAPACK.Test_${variant} COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindLAPACK/Test"
    "${CMake_BINARY_DIR}/Tests/FindLAPACK/Test_${variant}"
    ${build_generator_args}
    --build-project TestFindLAPACK
    --build-options ${build_options} ${alt_compiler} -DBLA_VENDOR=${vendor}
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
endforeach()

foreach(variant IN LISTS CMake_TEST_FindLAPACK_STATIC)
  if(variant MATCHES "^([^.]+)\\.(.*)$")
    set(vendor "${CMAKE_MATCH_1}")
    set(alt_compiler "-DCMAKE_C_COMPILER=${CMAKE_MATCH_2}")
  else()
    set(vendor "${variant}")
    set(alt_compiler "")
  endif()
  add_test(NAME FindLAPACK.Test_${variant}_Static COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindLAPACK/Test"
    "${CMake_BINARY_DIR}/Tests/FindLAPACK/Test_${variant}_Static"
    ${build_generator_args}
    --build-project TestFindLAPACK
    --build-options ${build_options} ${alt_compiler} -DBLA_VENDOR=${vendor} -DBLA_STATIC=ON
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
endforeach()
