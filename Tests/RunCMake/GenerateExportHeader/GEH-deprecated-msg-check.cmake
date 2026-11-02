# The build of an object library of the deprecated_msg directory must warn
# with the message of each deprecation macro of the library it uses.
string(REGEX REPLACE "^deprecated_msg_([a-z]+).*" "\\1" library
  "${deprecated_msg_target}")
foreach(macro IN ITEMS DEPRECATED_MSG DEPRECATED_EXPORT_MSG
    DEPRECATED_NO_EXPORT_MSG)
  if(NOT actual_stdout MATCHES "${library} ${macro}")
    string(APPEND RunCMake_TEST_FAILED
      "Building ${deprecated_msg_target} does not warn with the message "
      "\"${library} ${macro}\".\n")
  endif()
endforeach()
