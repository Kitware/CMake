include(RunCMake)

function(run_single_config_test label config exclude_from_all_value expectation)
    set(case single-config)
    message("-- Starting ${case} test: ${label}")
    set(full_case_name "${case}-build-${config}")
    set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/${full_case_name}/")
    run_cmake_with_options(${case}
        -DCMAKE_BUILD_TYPE=${config}
        -DTOOL_EXCLUDE_FROM_ALL=${exclude_from_all_value})
    set(RunCMake_TEST_NO_CLEAN 1)
    include(${RunCMake_TEST_BINARY_DIR}/target_files_${config}.cmake)
    run_cmake_command(${case}-build ${CMAKE_COMMAND} --build . --config ${config})
endfunction()

run_single_config_test("explicitly not excluded" Debug 0 "should_exist")
run_single_config_test("excluded" Debug 1 "should_not_exist")

if(RunCMake_GENERATOR MATCHES "^(Xcode|Visual Studio)")
    run_cmake(error-on-mixed-config)
else()
    run_single_config_test("explicitly not excluded with genex"
        Release $<CONFIG:Debug> "should_exist")
    run_single_config_test("excluded with genex"
        Debug $<CONFIG:Debug> "should_not_exist")
endif()
