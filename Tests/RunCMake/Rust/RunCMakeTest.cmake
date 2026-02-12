include(RunCMake)

run_cmake(Enable)
run_cmake(EnableExperimental)

block()
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/Editions-build)
    run_cmake(Editions)
    set(RunCMake_TEST_NO_CLEAN 1)
    run_cmake_command(Editions-build ${CMAKE_COMMAND} --build . --config Debug)
endblock()
