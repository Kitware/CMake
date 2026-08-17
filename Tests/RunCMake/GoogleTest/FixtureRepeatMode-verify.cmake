# Verify whether the generated discovery scripts carry the mode chosen by
# policy CMP0224 through to the tests they create.  ${expect_mode} is the
# mode expected, or empty if nothing should be recorded.  A multi-config
# generator writes one script per configuration.
set(property "_CMAKE_DEFAULT_FIXTURE_REPEAT_MODE")
file(GLOB discovery_scripts
  "${RunCMake_TEST_BINARY_DIR}/example_*_discovery.cmake")
if(NOT discovery_scripts)
  set(RunCMake_TEST_FAILED "No test discovery script was generated.")
  return()
endif()
foreach(discovery_script IN LISTS discovery_scripts)
  file(READ "${discovery_script}" content)
  if(expect_mode STREQUAL "")
    if(content MATCHES "${property}")
      set(RunCMake_TEST_FAILED
        "${property} should not appear in ${discovery_script}:\n${content}")
    endif()
  elseif(NOT content MATCHES "${property}[^A-Za-z_]+${expect_mode}")
    set(RunCMake_TEST_FAILED
      "${property} ${expect_mode} missing from ${discovery_script}:\n${content}")
  endif()
endforeach()
