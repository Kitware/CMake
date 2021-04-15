foreach(vendor IN LISTS CMake_TEST_FindLAPACK)
  add_test(NAME FindLAPACK.Test_${vendor} COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindLAPACK/Test"
    "${CMake_BINARY_DIR}/Tests/FindLAPACK/Test_${vendor}"
    ${build_generator_args}
    --build-project TestFindLAPACK
    --build-options ${build_options} -DBLA_VENDOR=${vendor}
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
endforeach()
