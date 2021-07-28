cmake_minimum_required(VERSION ${CMAKE_VERSION} FATAL_ERROR)

function(findExpectedFile FILE_NO RESULT_VAR GLOBING_EXPR_VAR)
  if(NOT DEFINED EXPECTED_FILE_${FILE_NO}) # explicit file name regex was not provided - construct one from other data
    # set defaults if parameters are not provided
    if(NOT DEFINED EXPECTED_FILE_${FILE_NO}_NAME)
      string(TOLOWER "${RunCMake_TEST_FILE_PREFIX}" EXPECTED_FILE_${FILE_NO}_NAME)
    endif()
    if(NOT DEFINED EXPECTED_FILE_${FILE_NO}_VERSION)
      set(EXPECTED_FILE_${FILE_NO}_VERSION "0.1.1")
      set(EXPECTED_FILE_${FILE_NO}_VERSION "0.1.1" PARENT_SCOPE)
    endif()

    getPackageNameGlobexpr("${EXPECTED_FILE_${FILE_NO}_NAME}"
      "${EXPECTED_FILE_${FILE_NO}_COMPONENT}" "${EXPECTED_FILE_${FILE_NO}_VERSION}"
      "${EXPECTED_FILE_${FILE_NO}_REVISION}" "${FILE_NO}" "EXPECTED_FILE_${FILE_NO}")
  endif()

  file(GLOB found_file_ RELATIVE "${bin_dir}" "${EXPECTED_FILE_${FILE_NO}}")

  set(${RESULT_VAR} "${found_file_}" PARENT_SCOPE)
  set(${GLOBING_EXPR_VAR} "${EXPECTED_FILE_${FILE_NO}}" PARENT_SCOPE)
endfunction()

include("${config_file}")
include("${src_dir}/${GENERATOR_TYPE}/Helpers.cmake")

file(READ "${bin_dir}/test_output.txt" output)
file(READ "${bin_dir}/test_error.txt" error)
file(READ "${config_file}" config_file_content)

string(REPLACE "\n" "\n cpack-out> " cpack_out "\n${output}")
string(REPLACE "\n" "\n cpack-err> " cpack_err "\n${error}")
string(REPLACE "\n" "\n cpack-res> " cpack_res "\n${PACKAGING_RESULT}")
string(REPLACE "\n" "\n cpack-cfg> " cpack_cfg "\n${config_file_content}")
string(CONCAT output_error_message
  "CPack output:${cpack_out}\n"
  "CPack error:${cpack_err}\n"
  "CPack result:${cpack_res}\n"
  "CPack config file:${cpack_cfg}"
  )

# generate default expected files data
include("${src_dir}/tests/${RunCMake_TEST_FILE_PREFIX}/ExpectedFiles.cmake")

# check that expected generated files exist and contain expected content
if(NOT EXPECTED_FILES_COUNT EQUAL 0)
  foreach(file_no_ RANGE 1 ${EXPECTED_FILES_COUNT})
    findExpectedFile("${file_no_}" "FOUND_FILE_${file_no_}"
      "EXPECTED_FILE_${file_no_}")
    list(APPEND foundFiles_ "${FOUND_FILE_${file_no_}}")
    list(LENGTH FOUND_FILE_${file_no_} foundFilesCount_)

    if(foundFilesCount_ EQUAL 1)
      unset(PACKAGE_CONTENT)

      if(DEFINED EXPECTED_FILE_CONTENT_${file_no_})
        getPackageContent("${bin_dir}/${FOUND_FILE_${file_no_}}" "PACKAGE_CONTENT")

        string(REGEX MATCH "${EXPECTED_FILE_CONTENT_${file_no_}}"
            expected_content_list "${PACKAGE_CONTENT}")
      else() # use content list
        getPackageContentList("${bin_dir}/${FOUND_FILE_${file_no_}}" "PACKAGE_CONTENT")
        set(EXPECTED_FILE_CONTENT_${file_no_} "${EXPECTED_FILE_CONTENT_${file_no_}_LIST}")
        toExpectedContentList("${file_no_}" "EXPECTED_FILE_CONTENT_${file_no_}")

        if(NOT PACKAGE_CONTENT STREQUAL "")
          list(SORT PACKAGE_CONTENT)
        endif()
        if(NOT EXPECTED_FILE_CONTENT_${file_no_} STREQUAL "")
          list(SORT EXPECTED_FILE_CONTENT_${file_no_})
        endif()

        if(PACKAGE_CONTENT STREQUAL EXPECTED_FILE_CONTENT_${file_no_})
          set(expected_content_list TRUE)
        else()
          set(expected_content_list FALSE)
        endif()
      endif()

      if(NOT expected_content_list)
        string(REPLACE "\n" "\n actual> " msg_actual "\n${PACKAGE_CONTENT}")
        string(REPLACE "\n" "\n expect> " msg_expected "\n${EXPECTED_FILE_CONTENT_${file_no_}}")
        message(FATAL_ERROR
          "Unexpected file content for file ${file_no_}!\n"
          "The content was:${msg_actual}\n"
          "which does not match:${msg_expected}\n"
          "${output_error_message}")
      endif()
    elseif(foundFilesCount_ EQUAL 0)
      message(FATAL_ERROR
        "Found no files for file ${file_no_}!\n"
        "Globbing expression:\n '${EXPECTED_FILE_${file_no_}}'\n"
        "${output_error_message}")
    else()
      message(FATAL_ERROR
        "Found more than one file for file ${file_no_}!\n"
        "Found files count '${foundFilesCount_}'.\n"
        "Files:\n '${FOUND_FILE_${file_no_}}'\n"
        "Globbing expression:\n '${EXPECTED_FILE_${file_no_}}'\n"
        "${output_error_message}")
    endif()
  endforeach()

  # check that there were no extra files generated
  foreach(all_files_glob_ IN LISTS ALL_FILES_GLOB)
    file(GLOB foundAll_ RELATIVE "${bin_dir}" "${all_files_glob_}")
    list(APPEND allFoundFiles_ ${foundAll_})
  endforeach()

  list(LENGTH foundFiles_ foundFilesCount_)
  list(LENGTH allFoundFiles_ allFoundFilesCount_)

  if(NOT foundFilesCount_ EQUAL allFoundFilesCount_)
    message(FATAL_ERROR
        "Found more files than expected!\n"
        "Found files:\n '${allFoundFiles_}'\n"
        "${output_error_message}")
  endif()

  # sanity check that we didn't accidentally list wrong files with our regular
  # expressions
  foreach(expected_ IN LISTS allFoundFiles_)
    list(FIND foundFiles_ "${expected_}" found_)

    if(found_ EQUAL -1)
      message(FATAL_ERROR
          "Expected files don't match found files!\n"
          "Found files:\n"
          " '${allFoundFiles_}'\n"
          "${output_error_message}")
    endif()
  endforeach()
else()
  # there should be no generated files present
  foreach(missing_file_glob_ IN LISTS ALL_FILES_GLOB)
    file(GLOB checkMissingFiles_ RELATIVE "${bin_dir}" "${missing_file_glob_}")

    if(checkMissingFiles_)
      message(FATAL_ERROR "Unexpected files found:\n '${checkMissingFiles_}'\n"
          "${output_error_message}")
    endif()
  endforeach()
endif()

# handle additional result verifications
if(EXISTS "${src_dir}/tests/${RunCMake_TEST_FILE_PREFIX}/VerifyResult.cmake")
  include("${src_dir}/tests/${RunCMake_TEST_FILE_PREFIX}/VerifyResult.cmake")
endif()

message(STATUS "${output}")
message("${error}")
