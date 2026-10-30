# Check the results that a dashboard run recorded in ${build_dir}.

cmake_policy(SET CMP0140 NEW)

function(get_dashboard_dir var)
  file(STRINGS "${build_dir}/Testing/TAG" tag LIMIT_COUNT 1)
  set(${var} "${build_dir}/Testing/${tag}" PARENT_SCOPE)
endfunction()

# Check that each named file was recorded.
function(check_dashboard_files)
  get_dashboard_dir(dir)
  foreach(file IN LISTS ARGN)
    if(NOT EXISTS "${dir}/${file}")
      string(APPEND RunCMake_TEST_FAILED
        "Dashboard file not recorded:\n  ${dir}/${file}\n")
    endif()
  endforeach()
  return(PROPAGATE RunCMake_TEST_FAILED)
endfunction()

# Check that a recorded file matches a regular expression.
function(check_dashboard_content file regex)
  get_dashboard_dir(dir)
  if(NOT EXISTS "${dir}/${file}")
    string(APPEND RunCMake_TEST_FAILED
      "Dashboard file not recorded:\n  ${dir}/${file}\n")
    return(PROPAGATE RunCMake_TEST_FAILED)
  endif()
  file(READ "${dir}/${file}" content)
  if(NOT content MATCHES "${regex}")
    string(APPEND RunCMake_TEST_FAILED
      "Dashboard file:\n  ${dir}/${file}\ndoes not match:\n  ${regex}\n")
  endif()
  return(PROPAGATE RunCMake_TEST_FAILED)
endfunction()

# Check the results of the passes and fails tests.
function(check_dashboard_tests file)
  check_dashboard_content(${file} "<Test Status=\"passed\">[^<]*<Name>passes</Name>")
  check_dashboard_content(${file} "<Test Status=\"failed\">[^<]*<Name>fails</Name>")
  return(PROPAGATE RunCMake_TEST_FAILED)
endfunction()

# Check the files that a build submitted to the local CDash server, which
# records each one in ${submit_log}.
function(check_dashboard_submitted build)
  set(expected ${ARGN})
  set(actual "")
  file(STRINGS "${submit_log}" records)
  foreach(record IN LISTS records)
    string(JSON record_build GET "${record}" build)
    if(record_build STREQUAL build)
      string(JSON file GET "${record}" file)
      string(REGEX REPLACE ".*___" "" file "${file}")
      list(APPEND actual "${file}")
    endif()
  endforeach()
  list(SORT expected)
  list(SORT actual)
  if(NOT actual STREQUAL expected)
    string(APPEND RunCMake_TEST_FAILED
      "Build '${build}' submitted:\n  ${actual}\nnot:\n  ${expected}\n")
  endif()
  return(PROPAGATE RunCMake_TEST_FAILED)
endfunction()

# Check the project and Authorization header of every file that a build
# submitted.
function(check_dashboard_submitted_to build project authorization)
  file(STRINGS "${submit_log}" records)
  foreach(record IN LISTS records)
    string(JSON record_build GET "${record}" build)
    if(record_build STREQUAL build)
      string(JSON record_project GET "${record}" project)
      string(JSON record_authorization GET "${record}" authorization)
      if(NOT record_project STREQUAL project
          OR NOT record_authorization STREQUAL authorization)
        string(APPEND RunCMake_TEST_FAILED
          "Build '${build}' submitted to project '${record_project}' "
          "with authorization '${record_authorization}'.\n")
      endif()
    endif()
  endforeach()
  return(PROPAGATE RunCMake_TEST_FAILED)
endfunction()
