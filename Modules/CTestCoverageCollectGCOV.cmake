#.rst:
# CTestCoverageCollectGCOV
# ------------------------
#
# This module provides the function ``ctest_coverage_collect_gcov``.
# The function will run gcov on the .gcda files in a binary tree and then
# package all of the .gcov files into a tar file with a data.json that
# contains the source and build directories for CDash to use in parsing
# the coverage data. In addtion the Labels.json files for targets that
# have coverage information are also put in the tar file for CDash to
# asign the correct labels. This file can be sent to a CDash server for
# display with the
# :command:`ctest_submit(CDASH_UPLOAD)` command.
#
# .. command:: cdash_coverage_collect_gcov
#
#   ::
#
#     ctest_coverage_collect_gcov(TARBALL <tarfile>
#       [SOURCE <source_dir>][BUILD <build_dir>]
#       [GCOV_COMMAND <gcov_command>]
#       )
#
#   Run gcov and package a tar file for CDash.  The options are:
#
#   ``TARBALL <tarfile>``
#     Specify the location of the ``.tar`` file to be created for later
#     upload to CDash.  Relative paths will be interpreted with respect
#     to the top-level build directory.
#
#   ``SOURCE <source_dir>``
#     Specify the top-level source directory for the build.
#     Default is the value of :variable:`CTEST_SOURCE_DIRECTORY`.
#
#   ``BUILD <build_dir>``
#     Specify the top-level build directory for the build.
#     Default is the value of :variable:`CTEST_BINARY_DIRECTORY`.
#
#   ``GCOV_COMMAND <gcov_command>``
#     Specify the full path to the ``gcov`` command on the machine.
#     Default is the value of :variable:`CTEST_COVERAGE_COMMAND`.

#=============================================================================
# Copyright 2014-2015 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)
include(CMakeParseArguments)
function(ctest_coverage_collect_gcov)
  set(options "")
  set(oneValueArgs TARBALL SOURCE BUILD GCOV_COMMAND)
  set(multiValueArgs "")
  cmake_parse_arguments(GCOV  "${options}" "${oneValueArgs}"
    "${multiValueArgs}" "" ${ARGN} )
  if(NOT DEFINED GCOV_TARBALL)
    message(FATAL_ERROR
      "TARBALL must be specified. for ctest_coverage_collect_gcov")
  endif()
  if(NOT DEFINED GCOV_SOURCE)
    set(source_dir "${CTEST_SOURCE_DIRECTORY}")
  else()
    set(source_dir "${GCOV_SOURCE}")
  endif()
  if(NOT DEFINED GCOV_BUILD)
    set(binary_dir "${CTEST_BINARY_DIRECTORY}")
  else()
    set(binary_dir "${GCOV_BUILD}")
  endif()
  if(NOT DEFINED GCOV_GCOV_COMMAND)
    set(gcov_command "${CTEST_COVERAGE_COMMAND}")
  else()
    set(gcov_command "${GCOV_GCOV_COMMAND}")
  endif()
  # run gcov on each gcda file in the binary tree
  set(gcda_files)
  set(label_files)
  # look for gcda files in the target directories
  # could do a glob from the top of the binary tree but
  # this will be faster and only look where the files will be
  file(STRINGS "${binary_dir}/CMakeFiles/TargetDirectories.txt" target_dirs)
  foreach(target_dir ${target_dirs})
    file(GLOB_RECURSE gfiles RELATIVE ${binary_dir} "${target_dir}/*.gcda")
    list(LENGTH gfiles len)
    # if we have gcda files then also grab the labels file for that target
    if(${len} GREATER 0)
      file(GLOB_RECURSE lfiles RELATIVE ${binary_dir}
        "${target_dir}/Labels.json")
      list(APPEND gcda_files ${gfiles})
      list(APPEND label_files ${lfiles})
    endif()
  endforeach()
  # return early if no coverage files were found
  list(LENGTH gcda_files len)
  if(len EQUAL 0)
    message("ctest_coverage_collect_gcov: No .gcda files found, "
      "ignoring coverage request.")
    return()
  endif()
  # setup the dir for the coverage files
  set(coverage_dir "${binary_dir}/Testing/CoverageInfo")
  file(MAKE_DIRECTORY  "${coverage_dir}")
  # call gcov on each .gcda file
  foreach (gcda_file ${gcda_files})
    # get the directory of the gcda file
    get_filename_component(gcda_file ${binary_dir}/${gcda_file} ABSOLUTE)
    get_filename_component(gcov_dir ${gcda_file} DIRECTORY)
    # run gcov, this will produce the .gcov file in the current
    # working directory
    execute_process(COMMAND
      ${gcov_command} -b -o ${gcov_dir} ${gcda_file}
      OUTPUT_VARIABLE out
      WORKING_DIRECTORY ${coverage_dir})
  endforeach()
  # create json file with project information
  file(WRITE ${coverage_dir}/data.json
    "{
    \"Source\": \"${source_dir}\",
    \"Binary\": \"${binary_dir}\"
}")
  # collect the gcov files
  set(gcov_files)
  file(GLOB_RECURSE gcov_files RELATIVE ${binary_dir} "${coverage_dir}/*.gcov")
  # tar up the coverage info with the same date so that the md5
  # sum will be the same for the tar file independent of file time
  # stamps
  string(REPLACE ";" "\n" gcov_files "${gcov_files}")
  string(REPLACE ";" "\n" label_files "${label_files}")
  file(WRITE "${coverage_dir}/coverage_file_list.txt"
    "${gcov_files}
${coverage_dir}/data.json
${label_files}
")
  execute_process(COMMAND
    ${CMAKE_COMMAND} -E tar cvfj ${GCOV_TARBALL}
    "--mtime=1970-01-01 0:0:0 UTC"
    --files-from=${coverage_dir}/coverage_file_list.txt
    WORKING_DIRECTORY ${binary_dir})
endfunction()
