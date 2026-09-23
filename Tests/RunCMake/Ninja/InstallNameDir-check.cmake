# The install-name directory is target data embedded in the binary, so it
# must keep its forward slashes on every host.
set(build_ninja "${RunCMake_TEST_BINARY_DIR}/build.ninja")
file(READ "${build_ninja}" content)
foreach(expected "INSTALLNAME_DIR = @rpath/\n" "INSTALLNAME_DIR = /custom/dir/\n")
  string(FIND "${content}" "${expected}" pos)
  if(pos EQUAL -1)
    string(STRIP "${expected}" expected)
    string(APPEND RunCMake_TEST_FAILED
      "${build_ninja} does not contain the line:\n  ${expected}\n")
  endif()
endforeach()
