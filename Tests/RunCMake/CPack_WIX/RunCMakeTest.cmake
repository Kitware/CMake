include(RunCPack)

set(env_PATH "$ENV{PATH}")

set(RunCPack_GENERATORS WIX)
set(RunCPack_GLOB *.msi)
set(RunCPack_VERIFY powershell -ExecutionPolicy Bypass -File ${CMAKE_CURRENT_LIST_DIR}/print-msi.ps1)

function(run_cpack_wix v)
  run_cpack(${v}-AppWiX SAMPLE AppWiX BUILD)
endfunction()

if(CMake_TEST_CPACK_WIX3)
  set(ENV{PATH} "${CMake_TEST_CPACK_WIX3};${env_PATH}")
  run_cpack_wix(3)
endif()
