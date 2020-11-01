
enable_language (${CHECK_LANGUAGE})

include(CheckLinkerFlag)

check_linker_flag(${CHECK_LANGUAGE} "LINKER:-L,/dir" VALID_LINKER_FLAG)
if(NOT VALID_LINKER_FLAG)
  message(SEND_ERROR "Test fail for valid linker flag.")
endif()

check_linker_flag(${CHECK_LANGUAGE} "LINKER:-D" INVALID_LINKER_FLAG)
if(INVALID_LINKER_FLAG)
  message(SEND_ERROR "Test fail for invalid linker flag.")
endif()
