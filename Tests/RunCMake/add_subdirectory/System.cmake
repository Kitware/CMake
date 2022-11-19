enable_language(CXX)

add_subdirectory(System SYSTEM)

function(check_target_system target expected_value)
  get_target_property(var ${target} SYSTEM)
  if ((var AND NOT expected_value) OR (NOT var AND expected_value))
    message(SEND_ERROR "\
The 'SYSTEM' property of ${target} should be ${expected_value}, \
but got ${var}")
  endif()
endfunction()

check_target_system(foo OFF)
check_target_system(bar ON)
check_target_system(zot ON)
check_target_system(subsub1foo OFF)
check_target_system(subsub1bar ON)
check_target_system(subsub1zot ON)
check_target_system(subsub2foo OFF)
check_target_system(subsub2bar ON)
check_target_system(subsub2zot ON)
