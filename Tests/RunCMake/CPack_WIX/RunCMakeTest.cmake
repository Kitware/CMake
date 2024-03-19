include(RunCPack)

set(RunCPack_GENERATORS WIX)

set(ENV{PATH} "${CMake_TEST_CPACK_WIX3};$ENV{PATH}")

run_cpack(AppWiX BUILD GLOB *.msi VERIFY powershell -ExecutionPolicy Bypass -File ${CMAKE_CURRENT_LIST_DIR}/print-msi.ps1)
