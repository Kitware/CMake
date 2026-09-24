# Unlike compilation, execution needs an available SYCL device.  Report a
# device-less machine as skipped, but do not hide compilation or runtime errors.
enable_testing()
function(sycl_add_runtime_test name target)
  add_test(NAME "${name}" COMMAND "${target}")
  set_tests_properties("${name}" PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 30)
endfunction()

function(sycl_not_supported reason)
  add_test(NAME NotSupported
    COMMAND "${CMAKE_COMMAND}" -E echo "SKIP: ${reason}")
  set_tests_properties(NotSupported PROPERTIES SKIP_REGULAR_EXPRESSION "^SKIP:")
endfunction()
