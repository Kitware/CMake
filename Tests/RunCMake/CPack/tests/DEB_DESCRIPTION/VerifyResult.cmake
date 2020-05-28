function(checkPackageDescription FILE EXPECTED_DESCRIPTION)
  getPackageInfo("${FILE}" "_file_info")
  string(UUID uuid NAMESPACE 00000000-0000-0000-0000-000000000000 TYPE SHA1)
  string(REPLACE ";" "${uuid}" _file_info "${_file_info}")
  string(REPLACE ";" "${uuid}" EXPECTED_DESCRIPTION "${EXPECTED_DESCRIPTION}")
  string(REPLACE "\n" ";" _file_info "${_file_info}")

  set(_actual_description)
  set(_parse_description FALSE)
  foreach(_line IN LISTS _file_info)
    if(_line MATCHES " Description:.*")
      set(_parse_description TRUE)
      list(APPEND _actual_description "${_line}")
    elseif(_parse_description)
      if(_line MATCHES " [A-Z][A-Za-z\-]+: .*")
        set(_parse_description FALSE)
      else()
        list(APPEND _actual_description "${_line}")
      endif()
    endif()
  endforeach()
  list(JOIN _actual_description "\n" _actual_description)

  if(NOT _actual_description STREQUAL EXPECTED_DESCRIPTION)
    set(_error "---[BEGIN Expected description]---\n${EXPECTED_DESCRIPTION}---[END Expected description]---\n")
    string(APPEND _error "---[BEGIN Actual description]---\n${_actual_description}---[END Actual description]---\n")
    string(REPLACE "${uuid}"  ";" _error "${_error}")
    message(FATAL_ERROR "${_error}")
  endif()
endfunction()

# ALERT The output of `dpkg -I *.deb` indented by one space
set(_expected_description [[ Description: This is the summary line
  This is the Debian package multiline description.
  .
  It must be formatted properly! Otherwise, the result `*.deb`
  package become broken and cant be installed!
  .
  It may contains `;` characters (even like this `;;;;`). Example:
  .
    - one;
    - two;
    - three;
  .
  ... and they are properly handled by the automatic description formatter!
  .
  See also: https://www.debian.org/doc/debian-policy/ch-controlfields.html#description]])

if(RunCMake_SUBTEST_SUFFIX STREQUAL "CPACK_NO_PACKAGE_DESCRIPTION")
  set(_expected_description [[ Description: This is the summary line]])
elseif(RunCMake_SUBTEST_SUFFIX STREQUAL "CPACK_COMPONENT_COMP_DESCRIPTION")
  set(_expected_description [[ Description: One line description]])
endif()

foreach(_file_no RANGE 1 ${EXPECTED_FILES_COUNT})
  checkPackageDescription("${FOUND_FILE_${_file_no}}" "${_expected_description}")
endforeach()

# kate: indent-width 2;
