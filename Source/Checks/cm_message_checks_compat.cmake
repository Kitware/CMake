# Supporting using the CHECK_... message modes if available
# and fall back to the older behavior if not
macro(cm_message_checks_compat description startVar passVar failVar)
  if(CMAKE_VERSION VERSION_GREATER 3.16.2019)
    set(${startVar} CHECK_START "${description}")
    set(${passVar}  CHECK_PASS)
    set(${failVar}  CHECK_FAIL)
  else()
    set(${startVar} STATUS "${description}")
    set(${passVar}  STATUS "${description} - ")
    set(${failVar}  STATUS "${description} - ")
  endif()
endmacro()
