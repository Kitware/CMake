message("Testing...")
message("FILE_PATH =       ${FILE_PATH}")
message("EXPECT_SYMLINK =  ${EXPECT_SYMLINK}")
message("EXPECT_ABSOLUTE = ${EXPECT_ABSOLUTE}")

if(NOT DEFINED FILE_PATH)
  message(FATAL_ERROR "FILE_PATH variable must be defined")
endif()

if(NOT EXISTS "${FILE_PATH}")
  message(FATAL_ERROR "File ${FILE_PATH} does not exist")
endif()

if(NOT DEFINED EXPECT_SYMLINK)
  message(FATAL_ERROR "EXPECT_SYMLINK must be defined")
endif()

if(EXPECT_SYMLINK)
  if(NOT DEFINED EXPECT_ABSOLUTE)
    message(FATAL_ERROR "EXPECT_ABSOLUTE variable must be defined")
  endif()

  if(NOT IS_SYMLINK "${FILE_PATH}")
    message(FATAL_ERROR "${FILE_PATH} must be a symlink")
  endif()

  file(READ_SYMLINK "${FILE_PATH}" TARGET_PATH)

  if(EXPECT_ABSOLUTE AND NOT IS_ABSOLUTE "${TARGET_PATH}")
    message(FATAL_ERROR "${FILE_PATH} must be an absolute symlink")
  elseif(NOT EXPECT_ABSOLUTE AND IS_ABSOLUTE "${TARGET_PATH}")
    message(FATAL_ERROR "${FILE_PATH} must be a relative symlink")
  endif()
else()
  if(IS_SYMLINK "${FILE_PATH}")
    message(FATAL_ERROR "${FILE_PATH} must NOT be a symlink")
  endif()
endif()
