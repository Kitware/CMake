include(RunCPack) # Uses sample projects from `../RunCPack/*`

set(env_PATH "$ENV{PATH}")

set(RunCPack_GENERATORS WIX)
set(RunCPack_GLOB *.msi)
set(RunCPack_VERIFY powershell -ExecutionPolicy Bypass -File ${CMAKE_CURRENT_LIST_DIR}/print-msi.ps1)

# Support non-interactive sessions (like CI).
if(NOT CMake_TEST_CPACK_WIX_VALIDATE)
  set(RunCMake_TEST_OPTIONS -DCPACK_WIX_LIGHT_EXTRA_FLAGS=-sval)
endif()

function(run_cpack_wix v)
  list(APPEND RunCMake_TEST_OPTIONS -DCPACK_WIX_VERSION=${v})
  run_cpack(${v}-AppWiX SAMPLE AppWiX BUILD)
endfunction()

function(run_cpack_wix_scope v scope)
  list(APPEND RunCMake_TEST_OPTIONS -DCPACK_WIX_VERSION=${v} -DCPACK_WIX_INSTALL_SCOPE=${scope})
  run_cpack(${v}-AppWiX-${scope} SAMPLE AppWiX BUILD)
endfunction()

if(CMake_TEST_CPACK_WIX3)
  set(ENV{PATH} "${CMake_TEST_CPACK_WIX3};${env_PATH}")
  run_cpack_wix(3)
  run_cpack_wix_scope(3 perUser)
  run_cpack_wix_scope(3 perUserOrMachine)
endif()

if(CMake_TEST_CPACK_WIX4)
  set(ENV{PATH} "${CMake_TEST_CPACK_WIX4};${env_PATH}")
  set(ENV{WIX_EXTENSIONS} "${CMake_TEST_CPACK_WIX4}")
  run_cpack_wix(4)
  run_cpack_wix_scope(4 perUser)
  run_cpack_wix_scope(4 perUserOrMachine)
  unset(ENV{WIX_EXTENSIONS})
endif()

block()
  list(APPEND RunCMake_TEST_OPTIONS -DCPACK_WIX_INSTALL_SCOPE=perUserOrMachine -DCPACK_WIX_ROOT_FOLDER_ID=LocalAppDataFolder)
  run_cpack(AppWiX-perUserOrMachine-ProfileRoot SAMPLE AppWiX NO_VERIFY)
endblock()
