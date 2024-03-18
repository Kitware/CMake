include(RunCPack)

set(RunCPack_GENERATORS WIX)

run_cpack(AppWiX BUILD GLOB *.msi VERIFY powershell -ExecutionPolicy Bypass -File ${CMAKE_CURRENT_LIST_DIR}/print-msi.ps1)
