
include ("${RunCMake_SOURCE_DIR}/check_errors.cmake")
unset (errors)

###############################################
## First test with a path defining all elements
###############################################
if (WIN32)
  set (path "C:/aa/bb/cc.ext1.ext2")
else()
  set (path "/aa/bb/cc.ext1.ext2")
endif()

cmake_path(GET path ROOT_NAME output)
if (WIN32)
  if (NOT output STREQUAL "C:")
    list (APPEND errors "ROOT_NAME returns bad data: ${output}")
  endif()
else()
  if (NOT output STREQUAL "")
    list (APPEND errors "ROOT_NAME returns bad data: ${output}")
  endif()
endif()

cmake_path(GET path ROOT_DIRECTORY output)
if (NOT output STREQUAL "/")
  list (APPEND errors "ROOT_DIRECTORY returns bad data: ${output}")
endif()

cmake_path(GET path ROOT_PATH output)
if (WIN32)
  if (NOT output STREQUAL "C:/")
    list (APPEND errors "ROOT_PATH returns bad data: ${output}")
  endif()
else()
  if (NOT output STREQUAL "/")
    list (APPEND errors "ROOT_PATH returns bad data: ${output}")
  endif()
endif()

cmake_path(GET path FILENAME output)
if (NOT output STREQUAL "cc.ext1.ext2")
  list (APPEND errors "FILENAME returns bad data: ${output}")
endif()

cmake_path(GET path EXTENSION output)
if (NOT output STREQUAL ".ext1.ext2")
  list (APPEND errors "EXTENSION returns bad data: ${output}")
endif()
cmake_path(GET path EXTENSION LAST_ONLY output)
if (NOT output STREQUAL ".ext2")
  list (APPEND errors "EXTENSION LAST_ONLY returns bad data: ${output}")
endif()

cmake_path(GET path STEM output)
if (NOT output STREQUAL "cc")
  list (APPEND errors "STEM returns bad data: ${output}")
endif()
cmake_path(GET path STEM LAST_ONLY output)
if (NOT output STREQUAL "cc.ext1")
  list (APPEND errors "STEM LAST_ONLY returns bad data: ${output}")
endif()

cmake_path(GET path RELATIVE_PART output)
if (NOT output STREQUAL "aa/bb/cc.ext1.ext2")
  list (APPEND errors "RELATIVE_PART returns bad data: ${output}")
endif()

cmake_path(GET path PARENT_PATH output)
if (WIN32)
  if (NOT output STREQUAL "C:/aa/bb")
    list (APPEND errors "PARENT_PATH returns bad data: ${output}")
  endif()
else()
  if (NOT output STREQUAL "/aa/bb")
    list (APPEND errors "PARENT_PATH returns bad data: ${output}")
  endif()
endif()

######################################
## second, tests with missing elements
######################################
set (path "aa/bb/")

cmake_path(GET path ROOT_NAME output)
if (NOT output STREQUAL "")
  list (APPEND errors "ROOT_NAME returns bad data: ${output}")
endif()

cmake_path(GET path ROOT_DIRECTORY output)
if (NOT output STREQUAL "")
  list (APPEND errors "ROOT_DIRECTORY returns bad data: ${output}")
endif()

cmake_path(GET path ROOT_PATH output)
if (NOT output STREQUAL "")
  list (APPEND errors "ROOT_PATH returns bad data: ${output}")
endif()

cmake_path(GET path FILENAME output)
if (NOT output STREQUAL "")
  list (APPEND errors "FILENAME returns bad data: ${output}")
endif()

cmake_path(GET path EXTENSION output)
if (NOT output STREQUAL "")
  list (APPEND errors "EXTENSION returns bad data: ${output}")
endif()

cmake_path(GET path STEM output)
if (NOT output STREQUAL "")
  list (APPEND errors "STEM returns bad data: ${output}")
endif()

cmake_path(GET path RELATIVE_PART output)
if (NOT output STREQUAL path)
  list (APPEND errors "RELATIVE_PART returns bad data: ${output}")
endif()

cmake_path(GET path PARENT_PATH output)
if (NOT output STREQUAL "aa/bb")
  list (APPEND errors "PARENT_PATH returns bad data: ${output}")
endif()

##################################
set (path "/aa/bb/")

cmake_path(GET path ROOT_NAME output)
if (NOT output STREQUAL "")
  list (APPEND errors "ROOT_NAME returns bad data: ${output}")
endif()

cmake_path(GET path ROOT_DIRECTORY output)
if (NOT output STREQUAL "/")
  list (APPEND errors "ROOT_DIRECTORY returns bad data: ${output}")
endif()

cmake_path(GET path ROOT_PATH output)
if (NOT output STREQUAL "/")
  list (APPEND errors "ROOT_PATH returns bad data: ${output}")
endif()

###################################
set (path "/")

cmake_path(GET path ROOT_NAME output)
if (NOT output STREQUAL "")
  list (APPEND errors "ROOT_NAME returns bad data: ${output}")
endif()

cmake_path(GET path ROOT_DIRECTORY output)
if (NOT output STREQUAL "/")
  list (APPEND errors "ROOT_DIRECTORY returns bad data: ${output}")
endif()

cmake_path(GET path ROOT_PATH output)
if (NOT output STREQUAL "/")
  list (APPEND errors "ROOT_PATH returns bad data: ${output}")
endif()

cmake_path(GET path FILENAME output)
if (NOT output STREQUAL "")
  list (APPEND errors "FILENAME returns bad data: ${output}")
endif()

cmake_path(GET path EXTENSION output)
if (NOT output STREQUAL "")
  list (APPEND errors "EXTENSION returns bad data: ${output}")
endif()

cmake_path(GET path STEM output)
if (NOT output STREQUAL "")
  list (APPEND errors "STEM returns bad data: ${output}")
endif()

cmake_path(GET path RELATIVE_PART output)
if (NOT output STREQUAL "")
  list (APPEND errors "RELATIVE_PART returns bad data: ${output}")
endif()

cmake_path(GET path PARENT_PATH output)
if (NOT output STREQUAL "/")
  list (APPEND errors "PARENT_PATH returns bad data: ${output}")
endif()

###################################
set (path ".file")

cmake_path(GET path FILENAME output)
if (NOT output STREQUAL ".file")
  list (APPEND errors "FILENAME returns bad data: ${output}")
endif()

cmake_path(GET path EXTENSION output)
if (NOT output STREQUAL "")
  list (APPEND errors "EXTENSION returns bad data: ${output}")
endif()

cmake_path(GET path STEM output)
if (NOT output STREQUAL ".file")
  list (APPEND errors "STEM returns bad data: ${output}")
endif()

###################################
set (path ".file.ext")

cmake_path(GET path FILENAME output)
if (NOT output STREQUAL ".file.ext")
  list (APPEND errors "FILENAME returns bad data: ${output}")
endif()

cmake_path(GET path EXTENSION output)
if (NOT output STREQUAL ".ext")
  list (APPEND errors "EXTENSION returns bad data: ${output}")
endif()
cmake_path(GET path EXTENSION LAST_ONLY output)
if (NOT output STREQUAL ".ext")
  list (APPEND errors "EXTENSION returns bad data: ${output}")
endif()

cmake_path(GET path STEM output)
if (NOT output STREQUAL ".file")
  list (APPEND errors "STEM returns bad data: ${output}")
endif()

###################################
set (path ".file.ext1.ext2")

cmake_path(GET path FILENAME output)
if (NOT output STREQUAL ".file.ext1.ext2")
  list (APPEND errors "FILENAME returns bad data: ${output}")
endif()

cmake_path(GET path EXTENSION output)
if (NOT output STREQUAL ".ext1.ext2")
  list (APPEND errors "EXTENSION returns bad data: ${output}")
endif()
cmake_path(GET path EXTENSION LAST_ONLY output)
if (NOT output STREQUAL ".ext2")
  list (APPEND errors "EXTENSION returns bad data: ${output}")
endif()

cmake_path(GET path STEM output)
if (NOT output STREQUAL ".file")
  list (APPEND errors "STEM returns bad data: ${output}")
endif()

##################################################
## tests for subcommands' handling of "." and ".."
##################################################
if (WIN32)
  set (dot "C:/aa/bb/.")
  set (dotdot "C:/ee/ff/..")
else()
  set (dot "/aa/bb/.")
  set (dotdot "/ee/ff/..")
endif()

cmake_path(GET dot FILENAME dot_out)
cmake_path(GET dotdot FILENAME dotdot_out)

if (NOT dot_out STREQUAL ".")
  list(APPEND errors "FILENAME returns bad data for '<path>/.': ${dot_out}")
endif()
if (NOT dotdot_out STREQUAL "..")
  list(APPEND errors "FILENAME returns bad data for '<path>/..': ${dotdot_out}")
endif()

cmake_path(GET dot STEM dot_out)
cmake_path(GET dotdot STEM dotdot_out)

if (NOT dot_out STREQUAL ".")
  list(APPEND errors "STEM returns bad data for '<path>/.': ${dot_out}")
endif()
if (NOT dotdot_out STREQUAL "..")
  list(APPEND errors "STEM returns bad data for '<path>/..': ${dotdot_out}")
endif()

cmake_path(GET dot STEM LAST_ONLY dot_out)
cmake_path(GET dotdot STEM LAST_ONLY dotdot_out)

if (NOT dot_out STREQUAL ".")
  list(APPEND errors
    "STEM LAST_ONLY returns bad data for '<path>/.': ${dot_out}")
endif()
if (NOT dotdot_out STREQUAL "..")
  list(APPEND errors
    "STEM LAST_ONLY returns bad data for '<path>/..': ${dotdot_out}")
endif()

cmake_path(GET dot EXTENSION dot_out)
cmake_path(GET dotdot EXTENSION dotdot_out)

if (NOT dot_out STREQUAL "")
  list(APPEND errors
    "EXTENSION returns bad data for '<path>/.': ${dot_out}")
endif()
if (NOT dotdot_out STREQUAL "")
  list(APPEND errors
    "EXTENSION returns bad data for '<path>/..': ${dotdot_out}")
endif()

cmake_path(GET dot EXTENSION LAST_ONLY dot_out)
cmake_path(GET dotdot EXTENSION LAST_ONLY dotdot_out)

if (NOT dot_out STREQUAL "")
  list(APPEND errors
    "EXTENSION LAST_ONLY returns bad data for '<path>/.': ${dot_out}")
endif()
if (NOT dotdot_out STREQUAL "")
  list(APPEND errors
    "EXTENSION LAST_ONLY returns bad data for '<path>/..': ${dotdot_out}")
endif()

check_errors (GET ${errors})
