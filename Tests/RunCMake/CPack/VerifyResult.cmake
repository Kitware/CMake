cmake_minimum_required(VERSION ${CMAKE_VERSION} FATAL_ERROR)

include("${config_file}")
include("${src_dir}/${GENERATOR_TYPE}/Helpers.cmake")

file(READ "${bin_dir}/test_output.txt" output)
file(READ "${bin_dir}/test_error.txt" error)
file(READ "${config_file}" config_file_content)

set(output_error_message
    "\nCPack output: '${output}'\nCPack error: '${error}';\nconfig file: '${config_file_content}'")

# check that expected generated files exist and contain expected content
include("${src_dir}/${GENERATOR_TYPE}/${RunCMake_TEST}-ExpectedFiles.cmake")

if(NOT EXPECTED_FILES_COUNT EQUAL 0)
  foreach(file_no_ RANGE 1 ${EXPECTED_FILES_COUNT})
    file(GLOB FOUND_FILE_${file_no_} RELATIVE "${bin_dir}" "${EXPECTED_FILE_${file_no_}}")
    set(foundFiles_ "${foundFiles_};${FOUND_FILE_${file_no_}}")
    list(LENGTH FOUND_FILE_${file_no_} foundFilesCount_)

    if(foundFilesCount_ EQUAL 1)
      unset(PACKAGE_CONTENT)
      getPackageContent("${bin_dir}/${FOUND_FILE_${file_no_}}" "PACKAGE_CONTENT")

      string(REGEX MATCH "${EXPECTED_FILE_CONTENT_${file_no_}}"
          expected_content_list "${PACKAGE_CONTENT}")

      if(NOT expected_content_list)
        message(FATAL_ERROR
          "Unexpected file content for file No. '${file_no_}'!\n"
          " Content: '${PACKAGE_CONTENT}'\n\n"
          " Expected: '${EXPECTED_FILE_CONTENT_${file_no_}}'"
          "${output_error_message}")
      endif()
    else()
      message(FATAL_ERROR
        "Found more than one file for file No. '${file_no_}'!"
        " Found files count '${foundFilesCount_}'."
        " Files: '${FOUND_FILE_${file_no_}}'"
        "${output_error_message}")
    endif()
  endforeach()

  # check that there were no extra files generated
  foreach(all_files_glob_ IN LISTS ALL_FILES_GLOB)
    file(GLOB foundAll_ RELATIVE "${bin_dir}" "${all_files_glob_}")
    set(allFoundFiles_ "${allFoundFiles_};${foundAll_}")
  endforeach()

  list(LENGTH foundFiles_ foundFilesCount_)
  list(LENGTH allFoundFiles_ allFoundFilesCount_)

  if(NOT foundFilesCount_ EQUAL allFoundFilesCount_)
    message(FATAL_ERROR
        "Found more files than expected! Found files: '${allFoundFiles_}'"
        "${output_error_message}")
  endif()

  # sanity check that we didn't accidentally list wrong files with our regular
  # expressions
  foreach(expected_ IN LISTS allFoundFiles_)
    list(FIND foundFiles_ "${expected_}" found_)

    if(found_ EQUAL -1)
      message(FATAL_ERROR
          "Expected files don't match found files! Found files:"
          " '${allFoundFiles_}'"
          "${output_error_message}")
    endif()
  endforeach()
else()
  # there should be no generated files present
  foreach(missing_file_glob_ IN LISTS ALL_FILES_GLOB)
    file(GLOB checkMissingFiles_ RELATIVE "${bin_dir}" "${missing_file_glob_}")

    if(checkMissingFiles_)
      message(FATAL_ERROR "Unexpected files found: '${checkMissingFiles_}'"
          "${output_error_message}")
    endif()
  endforeach()
endif()

# handle additional result verifications
if(EXISTS "${src_dir}/${GENERATOR_TYPE}/${RunCMake_TEST}-VerifyResult.cmake")
  include("${src_dir}/${GENERATOR_TYPE}/${RunCMake_TEST}-VerifyResult.cmake")
else()
  # by default only print out output and error so that they can be compared by
  # regex
  message(STATUS "${output}")
  message("${error}")
endif()
