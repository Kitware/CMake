cmake_policy(SET CMP0007 NEW)

include("${config_file}")
include("${src_dir}/${GENERATOR_TYPE}/Helpers.cmake")

# check that expected generated files exist and contain expected content
include("${src_dir}/${GENERATOR_TYPE}/${RunCMake_TEST}-ExpectedFiles.cmake")

if(NOT EXPECTED_FILES_COUNT EQUAL 0)
  foreach(file_no_ RANGE 1 ${EXPECTED_FILES_COUNT})
    file(GLOB foundFile_ RELATIVE "${bin_dir}" "${EXPECTED_FILE_${file_no_}}")
    set(foundFiles_ "${foundFiles_};${foundFile_}")
    list(LENGTH foundFile_ foundFilesCount_)

    if(foundFilesCount_ EQUAL 1)
      unset(PACKAGE_CONTENT)
      getPackageContent("${bin_dir}/${foundFile_}" "PACKAGE_CONTENT")

      string(REGEX MATCH "${EXPECTED_FILE_CONTENT_${file_no_}}"
          expected_content_list "${PACKAGE_CONTENT}")

      if(NOT expected_content_list)
        message(FATAL_ERROR
          "Unexpected file content for file No. '${file_no_}'!"
          " Content: '${PACKAGE_CONTENT}'")
      endif()
    else()
      message(FATAL_ERROR
        "Found more than one file for file No. '${file_no_}'!"
        " Found files count '${foundFilesCount_}'."
        " Files: '${foundFile_}'")
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
        "Found more files than expected! Found files: '${allFoundFiles_}'")
  endif()

  # sanity check that we didn't accidentaly list wrong files with our regular
  # expressions
  foreach(expected_ IN LISTS allFoundFiles_)
    list(FIND foundFiles_ "${expected_}" found_)

    if(found_ EQUAL -1)
      message(FATAL_ERROR
          "Expected files don't match found files! Found files:"
          " '${allFoundFiles_}'")
    endif()
  endforeach()
else()
  # there should be no generated files present
  foreach(missing_file_glob_ IN LISTS ALL_FILES_GLOB)
    file(GLOB checkMissingFiles_ RELATIVE "${bin_dir}" "${missing_file_glob_}")

    if(checkMissingFiles_)
      message(FATAL_ERROR "Unexpected files found: '${checkMissingFiles_}'")
    endif()
  endforeach()
endif()

file(READ "${bin_dir}/test_output.txt" output)
file(READ "${bin_dir}/test_error.txt" error)

# handle additional result verifications
if(EXISTS "${src_dir}/${GENERATOR_TYPE}/${RunCMake_TEST}-VerifyResult.cmake")
  include("${src_dir}/${GENERATOR_TYPE}/${RunCMake_TEST}-VerifyResult.cmake")
else()
  # by default only print out output and error so that they can be compared by
  # regex
  message(STATUS "${output}")
  message("${error}")
endif()
