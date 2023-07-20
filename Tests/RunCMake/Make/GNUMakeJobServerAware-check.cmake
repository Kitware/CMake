# This test verifies that the commands in the generated Makefiles contain the
# `+` prefix
function(check_for_plus_prefix target)
  set(file "${RunCMake_BINARY_DIR}/GNUMakeJobServerAware-build/${target}")
  file(READ "${file}" build_file)
  if(NOT "${build_file}" MATCHES [[\+]])
    message(FATAL_ERROR "The file ${file} does not contain the expected prefix in the custom command.")
  endif()
endfunction()

check_for_plus_prefix("CMakeFiles/dummy.dir/build.make")
check_for_plus_prefix("CMakeFiles/dummy2.dir/build.make")
