if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
  include("${RunCMake_TEST_BINARY_DIR}/default/CPackConfig.cmake")
  set(cpack_dir "${RunCMake_TEST_BINARY_DIR}/default/_CPack_Packages/${CPACK_TOPLEVEL_TAG}")
  set(contents [[Debug
Release
]])

  file(GLOB dirs RELATIVE "${cpack_dir}" "${cpack_dir}/*")
  foreach(dir IN LISTS dirs)
    set(configs_file "${cpack_dir}/${dir}/${CPACK_PACKAGE_FILE_NAME}/configs.txt")
    file(READ "${configs_file}" actual_contents)
    if(NOT contents STREQUAL actual_contents)
      string(REPLACE "\n" "\n  " contents_formatted "${contents}")
      string(REPLACE "\n" "\n  " actual_contents_formatted "${actual_contents}")
      string(APPEND RunCMake_TEST_FAILED "Expected contents of ${configs_file}:\n  ${contents_formatted}\nActual contents:\n  ${actual_contents_formatted}\n")
    endif()
  endforeach()
endif()
