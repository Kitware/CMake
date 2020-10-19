set(windows_cmd "a \"b c\" 'd e' \";\" \\ \"c:\\windows\\path\\\\\" \\\"")
set(windows_exp "a;b c;'d;e';\;;\\;c:\\windows\\path\\;\"")
separate_arguments(windows_out WINDOWS_COMMAND "${windows_cmd}")

if(NOT "${windows_out}" STREQUAL "${windows_exp}")
  message(FATAL_ERROR "separate_arguments windows-style failed.  "
    "Expected\n  [${windows_exp}]\nbut got\n  [${windows_out}]\n")
endif()

separate_arguments(empty_out WINDOWS_COMMAND)
if(NOT empty_out STREQUAL "")
  message(FATAL_ERROR "separate_arguments windows-style failed on no arguments")
endif()
