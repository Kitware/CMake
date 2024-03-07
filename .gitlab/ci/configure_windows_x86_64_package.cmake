# CPack package file name component for this platform.
set(CPACK_SYSTEM_NAME "windows-x86_64" CACHE STRING "")

# Tell WiX to package for this architecture.
set(CPACK_WIX_ARCHITECTURE "x64" CACHE STRING "")

# Use APIs from at most Windows 7
set(CMAKE_C_FLAGS "-D_WIN32_WINNT=0x601 -DNTDDI_VERSION=0x06010000" CACHE STRING "")
set(CMAKE_CXX_FLAGS "-GR -EHsc -D_WIN32_WINNT=0x601 -DNTDDI_VERSION=0x06010000" CACHE STRING "")
set(CMAKE_EXE_LINKER_FLAGS "-machine:x64 -subsystem:console,6.01" CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_windows_package_common_x86.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/configure_windows_package_common.cmake")
