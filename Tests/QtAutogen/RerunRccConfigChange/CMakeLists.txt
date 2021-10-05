cmake_minimum_required(VERSION 3.16)
project(RerunRccConfigChange)
include("../AutogenCoreTest.cmake")

# Tests rcc rebuilding after a configuration change

# Dummy executable to generate a clean target
add_executable(dummy dummy.cpp)

# When a .qrc or a file listed in a .qrc file changes,
# the target must be rebuilt
set(rccDepSD "${CMAKE_CURRENT_SOURCE_DIR}/RccConfigChange")
set(rccDepBD "${CMAKE_CURRENT_BINARY_DIR}/RccConfigChange")

# Rebuild macro
macro(rebuild CFG)
  message(STATUS "Rebuilding rccConfigChange in ${CFG} configuration.")
  execute_process(
    COMMAND "${CMAKE_COMMAND}" --build . --config "${CFG}"
    WORKING_DIRECTORY "${rccDepBD}"
    RESULT_VARIABLE result)
  if (result)
    message(FATAL_ERROR "${CFG} build of rccConfigChange failed.")
  else()
    message(STATUS "${CFG} build of rccConfigChange finished.")
  endif()
endmacro()

# Initial build
try_compile(RCC_DEPENDS
  "${rccDepBD}"
  "${rccDepSD}"
  RccConfigChange
  CMAKE_FLAGS "-DQT_TEST_VERSION=${QT_TEST_VERSION}"
              "-DCMAKE_AUTOGEN_VERBOSE=${CMAKE_AUTOGEN_VERBOSE}"
              "-DCMAKE_PREFIX_PATH:STRING=${CMAKE_PREFIX_PATH}"
              "-DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}"
  OUTPUT_VARIABLE output
)
if (NOT RCC_DEPENDS)
  message(FATAL_ERROR "Initial build of rccConfigChange failed. Output: ${output}")
endif()

# Rebuild: Release
rebuild(Release)

# Rebuild: Debug
rebuild(Debug)
