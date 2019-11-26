file(READ ${RunCMake_TEST_BINARY_DIR}/cmake_install.cmake install_script)
#message(STATUS ${install_script})

set(wsnl " *[\n\r]+ *") # whitespace + single newline + whitespace
set(wssl " *[\n\r]+[^\n\r]*[\n\r]+ *") # ws nl skipline nl ws
string(CONCAT prefix [[file\(RPATH_CHANGE]])
set(_msg "cmake_install.cmake does not match ")

macro(check)
  if(NOT install_script MATCHES "${regex}")
    message(STATUS "${test} - check \"${target}\" - FAILED:")
    string(CONCAT RunCMake_TEST_FAILED "${_msg}" ">>>${regex}<<<")
    return()
  else()
    message(STATUS "${test} - check \"${target}\" - PASSED")
  endif()
endmacro()

macro(skip_without_rpath_change_rule)
# Not all platforms generate a file(RPATH_CHANGE) rule
  if(NOT install_script MATCHES [[file\(RPATH_CHANGE]])
    # Sanity check against a platform known to generate a file(RPATH_CHANGE) rule
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
      message(FATAL_ERROR "Expected generated file(RPATH_CHANGE) rule on platform Linux.")
    else()
      message(STATUS "${test} - All checks skipped. No file(RPATH_CHANGE) rule found on this platform.")
      return()
    endif()
  endif()
endmacro()
