set(framework-dir "${RunCMake_TEST_BINARY_DIR}/Framework.framework")
set(framework-resources "${framework-dir}/Resources")
set(framework-resource-file "${framework-resources}/res.txt")
set(framework-library "${framework-dir}/Framework")
set(framework-versions "${framework-dir}/Versions")
set(plist-file "${framework-resources}/Info.plist")
set(framework-header "${framework-dir}/Headers/foo.h")

if(NOT IS_DIRECTORY ${framework-dir})
  message(SEND_ERROR "Framework not found at ${framework-dir}")
endif()

if(NOT EXISTS ${plist-file})
  message(SEND_ERROR "plist file not found at ${plist-file}")
endif()

if(NOT EXISTS ${framework-library})
  message(SEND_ERROR "Framework library not found at ${framework-library}")
endif()

if(NOT EXISTS ${framework-resource-file})
  message(SEND_ERROR "Framework resource file not found at ${framework-resource-file}")
endif()

if(NOT EXISTS ${framework-versions})
  message(SEND_ERROR "Framework versions not found at ${framework-versions}")
endif()

if(NOT EXISTS ${framework-resources})
  message(SEND_ERROR "Framework Resources not found at ${framework-resources}")
endif()

if(NOT EXISTS ${framework-header})
  message(SEND_ERROR "Framework header file not found at ${framework-header}")
endif()
