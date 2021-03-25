enable_testing()
add_test(testa ${CMAKE_COMMAND} -E echo testa)
add_test(testb ${CMAKE_COMMAND} -E echo testb)
add_test(testc ${CMAKE_COMMAND} -E echo testc)
add_test(testd ${CMAKE_COMMAND} -E echo testd)

set_tests_properties(testa testb testc testd
                     PROPERTIES LABELS "echo")
set_property(TEST testa testb
             APPEND PROPERTY LABELS ab)
set_property(TEST testb testc
             APPEND PROPERTY LABELS bc)
set_property(TEST testc testd
             APPEND PROPERTY LABELS cd)

add_test(test-env ${CMAKE_COMMAND} -E environment | sort)
set_tests_properties(test-env PROPERTIES LABELS "env")

add_test(NAME debug-only
         COMMAND ${CMAKE_COMMAND} -E echo debug-only
         CONFIGURATIONS Debug)
add_test(NAME release-only
         COMMAND ${CMAKE_COMMAND} -E echo release-only
         CONFIGURATIONS Release)

set_tests_properties(debug-only release-only
                     PROPERTIES LABELS "config")
