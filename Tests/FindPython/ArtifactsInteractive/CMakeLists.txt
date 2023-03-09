cmake_minimum_required(VERSION 3.5)

project(TestArtifactsInteractive LANGUAGES C)

set (components Interpreter Development)
if (CMake_TEST_FindPython_NumPy)
  list (APPEND components NumPy)
endif()

find_package(Python3 REQUIRED COMPONENTS ${components})

if (Python3_ARTIFACTS_INTERACTIVE)
  if (NOT DEFINED CACHE{Python3_EXECUTABLE}
      OR NOT DEFINED CACHE{Python3_LIBRARY} OR NOT DEFINED CACHE{Python3_INCLUDE_DIR}
      OR (CMake_TEST_FindPython_NumPy AND NOT DEFINED CACHE{Python3_NumPy_INCLUDE_DIR}))
    message (FATAL_ERROR "Python3_ARTIFACTS_INTERACTIVE=ON Failed.")
  endif()
else()
  if (DEFINED CACHE{Python3_EXECUTABLE}
      OR DEFINED CACHE{Python3_LIBRARY} OR DEFINED CACHE{Python3_INCLUDE_DIR}
      OR (CMake_TEST_FindPython_NumPy AND DEFINED CACHE{Python3_NumPy_INCLUDE_DIR}))
    message (FATAL_ERROR "Python3_ARTIFACTS_INTERACTIVE=OFF Failed.")
  endif()
endif()
