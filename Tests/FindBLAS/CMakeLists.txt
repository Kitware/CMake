foreach(variant IN LISTS CMake_TEST_FindBLAS)
  if(variant MATCHES "^([^.]+)\\.(.*)$")
    set(vendor "${CMAKE_MATCH_1}")
    set(alt_compiler "-DCMAKE_C_COMPILER=${CMAKE_MATCH_2}")
  else()
    set(vendor "${variant}")
    set(alt_compiler "")
  endif()
  if(vendor STREQUAL "All" AND CMake_TEST_FindBLAS_All)
    set(EXPECT_All "-DEXPECT_All=${CMake_TEST_FindBLAS_All}")
  else()
    set(EXPECT_All "")
  endif()
  add_test(NAME FindBLAS.Test_${variant} COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindBLAS/Test"
    "${CMake_BINARY_DIR}/Tests/FindBLAS/Test_${variant}"
    ${build_generator_args}
    --build-project TestFindBLAS
    --build-options ${build_options} ${alt_compiler} ${EXPECT_All} -DBLA_VENDOR=${vendor}
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
endforeach()

foreach(variant IN LISTS CMake_TEST_FindBLAS_STATIC)
  if(variant MATCHES "^([^.]+)\\.(.*)$")
    set(vendor "${CMAKE_MATCH_1}")
    set(alt_compiler "-DCMAKE_C_COMPILER=${CMAKE_MATCH_2}")
  else()
    set(vendor "${variant}")
    set(alt_compiler "")
  endif()
  if(vendor STREQUAL "All" AND CMake_TEST_FindBLAS_STATIC_All)
    set(EXPECT_All "-DEXPECT_All=${CMake_TEST_FindBLAS_STATIC_All}")
  else()
    set(EXPECT_All "")
  endif()
  add_test(NAME FindBLAS.Test_${variant}_Static COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindBLAS/Test"
    "${CMake_BINARY_DIR}/Tests/FindBLAS/Test_${variant}_Static"
    ${build_generator_args}
    --build-project TestFindBLAS
    --build-options ${build_options} ${alt_compiler} ${EXPECT_All} -DBLA_VENDOR=${vendor} -DBLA_STATIC=ON
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
endforeach()
