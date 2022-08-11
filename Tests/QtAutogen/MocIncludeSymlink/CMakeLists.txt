cmake_minimum_required(VERSION 3.16)
project(MocIncludeSymlink)
get_filename_component(AUTOGEN_CORE_TEST_CMAKE "../AutogenCoreTest.cmake" ABSOLUTE)
include("${AUTOGEN_CORE_TEST_CMAKE}")

#
# Tests if MocInclude can be build when
# - The source directory is a symbolic link
# - The build directory is a symbolic link
#

# -- Utility variables
set(CS_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(CB_DIR "${CMAKE_CURRENT_BINARY_DIR}")

# Absolute MocInclude path
get_filename_component(MocIncludePath "../MocInclude" ABSOLUTE)
message("MocIncludePath: ${MocIncludePath}")

# Use nested subdirectories to ensure relatives paths are correct as well
set(BUILD_DIR_NORMAL "${CB_DIR}/Build/Normal")
set(BUILD_DIR_LINKED "${CB_DIR}/Build/Linked")

set(SL_SOURCE_DIR "${CB_DIR}/SL_Source")
set(SL_BUILD_DIR "${CB_DIR}/SL_Build")

# -- Utility macros
function(makeSymLink origin link)
    message("Creating symbolic link\n  Link: ${link}\n  To:   ${origin}")
    file(CREATE_LINK ${origin} ${link} RESULT res SYMBOLIC)
    if(NOT (${res} STREQUAL "0"))
      message("Symlink creation failed.\n  Link: ${link}\n  To:   ${origin}\n  Result: ${res}")
    endif()
endfunction()

# -- Make source directory symlink
makeSymLink(${MocIncludePath} ${SL_SOURCE_DIR} linkResult)
if(NOT EXISTS ${SL_SOURCE_DIR})
  message("Directory symlink can't be created.  Skipping test.")
  return()
endif()

# -- Make normal build directory
file(REMOVE_RECURSE ${BUILD_DIR_NORMAL})
file(MAKE_DIRECTORY ${BUILD_DIR_NORMAL})

# -- Make linked build directory and symlink
file(REMOVE_RECURSE ${BUILD_DIR_LINKED})
file(MAKE_DIRECTORY ${BUILD_DIR_LINKED})
makeSymLink(${BUILD_DIR_LINKED} ${SL_BUILD_DIR} linkResult)
if(NOT EXISTS ${SL_BUILD_DIR})
  message("Directory symlink can't be created.  Skipping test.")
  return()
endif()


# -- Building
macro(buildMocInclude sourceDir binaryDir)
  message("Building MocInclude\n  - source dir: ${sourceDir}\n  - binary dir: ${binaryDir}\n")
  try_compile(result
    "${binaryDir}"
    "${sourceDir}"
    MocInclude
    CMAKE_FLAGS "-DQT_TEST_VERSION=${QT_TEST_VERSION}"
                "-DCMAKE_AUTOMOC_PATH_PREFIX=ON"
                "-DCMAKE_AUTOGEN_VERBOSE=${CMAKE_AUTOGEN_VERBOSE}"
                "-DCMAKE_PREFIX_PATH:STRING=${CMAKE_PREFIX_PATH}"
                "-DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}"
                "-DAUTOGEN_CORE_TEST_CMAKE:STRING=${AUTOGEN_CORE_TEST_CMAKE}"
    OUTPUT_VARIABLE output
  )
  if (result)
    message(STATUS "--- Build success. ---")
  else()
    message(STATUS "\n### Building MocInclude failed. ###\n\n--- Output ---\n${output}")
    message(FATAL_ERROR "--- Building MocInclude failed. End of output. ---")
  endif()
endmacro()

message("\nTry building with\n  - symbolic link source dir\n  - non symbolic build dir\n")
buildMocInclude(${SL_SOURCE_DIR} ${BUILD_DIR_NORMAL})

message("\nTry building with\n  - symbolic link source dir\n  - symbolic link build dir\n")
buildMocInclude(${SL_SOURCE_DIR} ${SL_BUILD_DIR})
