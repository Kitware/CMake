# The preset's binaryDir is "${sourceDir}/build", so Testing/ should be
# written there, not in the working directory used to invoke ctest.
file(GLOB configure_xml_file
  "${RunCMake_TEST_SOURCE_DIR}/build/Testing/*/Configure.xml")
if(configure_xml_file)
  file(READ "${configure_xml_file}" configure_xml)
  if(NOT configure_xml MATCHES "\"--preset\" \"my-preset\"")
    set(RunCMake_TEST_FAILED
        "Configure.xml does not contain the expected --preset argument")
  endif()
else()
  set(RunCMake_TEST_FAILED "Configure.xml not found in preset binaryDir")
endif()

set(cmakecache_file "${RunCMake_TEST_SOURCE_DIR}/build/CMakeCache.txt")
if(EXISTS "${cmakecache_file}")
  file(READ "${cmakecache_file}" cmakecache_txt)
  if(NOT cmakecache_txt MATCHES "MY_CUSTOM_VAR:STRING=this-gets-set")
    set(RunCMake_TEST_FAILED "CMakeCache.txt does not contain MY_CUSTOM_VAR")
  endif()
else()
  set(RunCMake_TEST_FAILED "CMakeCache.txt not found in preset binaryDir")
endif()
