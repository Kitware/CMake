# CPack package file name component for this platform.
set(CPACK_SYSTEM_NAME "windows-arm64" CACHE STRING "")

# Tell WiX to package for this architecture.
set(CPACK_WIX_ARCHITECTURE "arm64" CACHE STRING "")

# Use APIs from at most Windows 7
set(CMAKE_C_FLAGS "-D_WIN32_WINNT=0x0A00 -DNTDDI_VERSION=0x0A000008" CACHE STRING "")
set(CMAKE_CXX_FLAGS "-GR -EHsc -D_WIN32_WINNT=0x0A00 -DNTDDI_VERSION=0x0A000008" CACHE STRING "")
set(CMAKE_EXE_LINKER_FLAGS "-machine:arm64 -subsystem:console,6.02" CACHE STRING "")

set(qt "$ENV{CI_PROJECT_DIR}/.gitlab/qt")
set(qt_host "$ENV{CI_PROJECT_DIR}/.gitlab/qt-host")
set(CMAKE_PREFIX_PATH "${qt};${qt_host}" CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_windows_package_common.cmake")
