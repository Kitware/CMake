foreach(vendor IN LISTS CMake_TEST_FindBLAS)
  add_test(NAME FindBLAS.Test_${vendor} COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindBLAS/Test"
    "${CMake_BINARY_DIR}/Tests/FindBLAS/Test_${vendor}"
    ${build_generator_args}
    --build-project TestFindBLAS
    --build-options ${build_options} -DBLA_VENDOR=${vendor}
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
endforeach()

foreach(vendor IN LISTS CMake_TEST_FindBLAS_STATIC)
  add_test(NAME FindBLAS.Test_${vendor}_Static COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindBLAS/Test"
    "${CMake_BINARY_DIR}/Tests/FindBLAS/Test_${vendor}_Static"
    ${build_generator_args}
    --build-project TestFindBLAS
    --build-options ${build_options} -DBLA_VENDOR=${vendor} -DBLA_STATIC=ON
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
endforeach()
