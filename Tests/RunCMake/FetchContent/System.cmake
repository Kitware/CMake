enable_language(CXX)

include(FetchContent)

FetchContent_Declare(
  IncludesSystem
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/IncludesSystem
  SYSTEM
)
FetchContent_MakeAvailable(IncludesSystem)

FetchContent_Declare(
  IncludesNonSystem
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/IncludesNonSystem
)
FetchContent_MakeAvailable(IncludesNonSystem)

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
check_target_system(foononsys OFF)
check_target_system(barnonsys OFF)
