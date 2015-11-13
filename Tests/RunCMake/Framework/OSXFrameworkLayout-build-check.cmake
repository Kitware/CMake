set(framework-dir "${RunCMake_TEST_BINARY_DIR}/Framework.framework")
set(plist-file "${framework-dir}/Resources/Info.plist")
set(framework-library "${framework-dir}/Framework")
set(framework-versions "${framework-dir}/Versions")

if(NOT IS_DIRECTORY ${framework-dir})
  message(SEND_ERROR "Framework not found at ${framework-dir}")
endif()

if(NOT EXISTS ${plist-file})
  message(SEND_ERROR "plist file not found at ${plist-file}")
endif()

if(NOT EXISTS ${framework-library})
  message(SEND_ERROR "Framework library not found at ${framework-library}")
endif()

if(NOT EXISTS ${framework-versions})
  message(SEND_ERROR "Framework versions not found at ${framework-versions}")
endif()
