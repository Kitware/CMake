

set(expected_values "SHARED_LIB_MACRO" "INTERFACE_LIB_MACRO" "STATIC_LIB_MACRO")
function(checkAutoMocMacroNames FILE_PATH)
  message(STATUS "Checking for auto moc macro names in ${FILE_PATH}")
  file(READ ${FILE_PATH} FILE_CONTENT)
  string(JSON MOC_MACRO_NAMES_ARR GET ${FILE_CONTENT} MOC_MACRO_NAMES)
  # get the length of MOC_MACRO_NAMES in JSON
  string(JSON MOC_MACRO_NAMES_LENGTH LENGTH ${MOC_MACRO_NAMES_ARR})
  if(${MOC_MACRO_NAMES_LENGTH} EQUAL 0)
      message(FATAL_ERROR "MOC_MACRO_NAMES is empty")
  endif()
  message(STATUS "MOC_MACRO_NAMES: ${MOC_MACRO_NAMES_ARR}")

  math(EXPR last_index "${MOC_MACRO_NAMES_LENGTH} - 1")
  set(reverse_index ${last_index})
  foreach(expected_value IN LISTS expected_values)
    string(JSON element GET ${MOC_MACRO_NAMES_ARR} ${reverse_index})
    # check if element equals to expected value
    if(NOT ${element} STREQUAL ${expected_value})
      message(FATAL_ERROR "MOC_MACRO_NAMES is expected to contain ${expected_value} but contains ${element}")
    endif()
    math(EXPR reverse_index "${reverse_index} - 1")
  endforeach()
endfunction()

checkAutoMocMacroNames(${FILE_PATH})
