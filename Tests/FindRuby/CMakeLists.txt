if(CMake_TEST_FindRuby)

  # Looks for ruby >=1.9.9, which is true on any Ubuntu (that installs it) or macOS (> 10.9)
  add_test(NAME FindRuby.Test COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindRuby/Test"
    "${CMake_BINARY_DIR}/Tests/FindRuby/Test"
    ${build_generator_args}
    --build-project TestRuby
    --build-options ${build_options}
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )

  # Looks for ruby >= 50.1.0, which should logically fail
  add_test(NAME FindRuby.Fail COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindRuby/Fail"
    "${CMake_BINARY_DIR}/Tests/FindRuby/Fail"
    ${build_generator_args}
    --build-project TestRubyFail
    --build-options ${build_options}
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  set_tests_properties(FindRuby.Fail PROPERTIES
    PASS_REGULAR_EXPRESSION "Could NOT find Ruby.*(Required[ \n]+is[ \n]+at[ \n]+least[ \n]+version[ \n]*\"[0-9]+\\.[0-9]+\\.[0-9]+\")")

  # Looks for 1.9.9 EXACTLY, which unlike the "FindRuby" test above will fail on every machine
  # since this version doesn't exist (ruby goes from 1.9.3 to 2.0.0)
  add_test(NAME FindRuby.FailExact COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindRuby/FailExact"
    "${CMake_BINARY_DIR}/Tests/FindRuby/FailExact"
    ${build_generator_args}
    --build-project TestRubyFailExact
    --build-options ${build_options}
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
    )
  set_tests_properties(FindRuby.FailExact PROPERTIES
    PASS_REGULAR_EXPRESSION "Could NOT find Ruby: Found unsuitable version \".*\", but required is.*exact version \"[0-9]+\\.[0-9]+\\.[0-9]+\" \\(found .*\\)")

  # RVM specific test
  if(CMake_TEST_FindRuby_RVM)
    add_test(NAME FindRuby.Rvm COMMAND
      ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/FindRuby/Rvm"
      "${CMake_BINARY_DIR}/Tests/FindRuby/Rvm"
      ${build_generator_args}
      --build-project TestRVM
      --build-options ${build_options}
      --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
      )
  endif()
endif()
