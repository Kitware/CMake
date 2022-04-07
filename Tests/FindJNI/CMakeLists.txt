foreach(test IN ITEMS Minimal JVM AWT)
  add_test(NAME FindJNI.${test} COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindJNI/${test}"
    "${CMake_BINARY_DIR}/Tests/FindJNI/${test}"
    ${build_generator_args}
    --build-project Test${test}
    --build-options ${build_options}
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
endforeach()
