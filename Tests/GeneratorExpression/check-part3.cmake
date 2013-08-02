
include(${CMAKE_CURRENT_LIST_DIR}/check-common.cmake)

check(test_version_greater_1 "0")
check(test_version_greater_2 "1")
check(test_version_less_1 "0")
check(test_version_less_2 "1")
check(test_version_equal_1 "0")
check(test_version_equal_2 "1")

foreach(c debug release relwithdebinfo minsizerel)
  if(config AND NOT config STREQUAL NoConfig)
    if(NOT "${test_imported_${c}}" MATCHES "^;/imported2/include$"
        AND NOT "${test_imported_${c}}" MATCHES "^/imported1/include;$")
      message(SEND_ERROR "test_imported_${c} is not correct: ${test_imported_${c}}")
    endif()
  else()
    if(NOT "${test_imported_${c}}" MATCHES "^;$")
      message(SEND_ERROR "test_imported_${c} is not an empty list: ${test_imported_${c}}")
    endif()
  endif()
endforeach()

check(test_alias_file_exe "1")
check(test_alias_file_lib "1")
check(test_alias_target_name "1")
