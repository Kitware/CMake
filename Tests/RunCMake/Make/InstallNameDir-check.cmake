# The install-name directory is target data embedded in the binary, so it
# must keep its forward slashes on every host.
foreach(case "rpath_dir;@rpath/" "custom_dir;/custom/dir/")
  list(GET case 0 target)
  list(GET case 1 dir)
  # Generators without link scripts inline the link command in build.make.
  set(link_txt "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/${target}.dir/link.txt")
  if(NOT EXISTS "${link_txt}")
    set(link_txt "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/${target}.dir/build.make")
  endif()
  file(READ "${link_txt}" content)
  string(FIND "${content}" "-install_name ${dir}" pos)
  if(pos EQUAL -1)
    string(APPEND RunCMake_TEST_FAILED
      "${link_txt} does not contain '-install_name ${dir}':\n${content}")
  endif()
endforeach()
