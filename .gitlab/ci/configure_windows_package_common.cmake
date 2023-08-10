set(CMake_DOC_ARTIFACT_PREFIX "$ENV{CI_PROJECT_DIR}/build/install-doc" CACHE PATH "")

# Set up install destinations as expected by the packaging scripts.
set(CMAKE_DOC_DIR "doc/cmake" CACHE STRING "")

# Link C/C++ runtime library statically.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "")

# Enable cmake-gui.
set(BUILD_QtDialog "TRUE" CACHE BOOL "")
set(CMake_GUI_DISTRIBUTE_WITH_Qt_LGPL "3" CACHE STRING "")

# Disable ccmake.
set(BUILD_CursesDialog "OFF" CACHE BOOL "")

set(CMake_TEST_BOOTSTRAP OFF CACHE BOOL "")
set(CMake_TEST_Java OFF CACHE BOOL "")
set(CMake_TEST_Qt5 OFF CACHE BOOL "")
set(CMake_TEST_Qt6 OFF CACHE BOOL "")
set(Python_FIND_REGISTRY NEVER CACHE STRING "")

set(CMake_CPACK_CUSTOM_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/CMakeCPack.cmake" CACHE FILEPATH "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_common.cmake")
