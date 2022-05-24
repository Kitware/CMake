include(FetchContent)

set(CMAKE_EP_GIT_REMOTE_UPDATE_STRATEGY AAAA)
set(CMAKE_TLS_VERIFY BBBB)
set(CMAKE_TLS_CAINFO CCCC)
set(CMAKE_NETRC DDDD)
set(CMAKE_NETRC_FILE EEEE)

FetchContent_Declare(PassThrough
  DOWNLOAD_COMMAND ${CMAKE_COMMAND} -E echo "Download command executed"
)
FetchContent_Populate(PassThrough)

set(gen_file ${FETCHCONTENT_BASE_DIR}/passthrough-subbuild/CMakeLists.txt)
if(NOT EXISTS ${gen_file})
  message(FATAL_ERROR "File does not exist: ${gen_file}")
endif()
file(READ ${gen_file} contents)

if(NOT contents MATCHES "CMAKE_EP_GIT_REMOTE_UPDATE_STRATEGY \\[==\\[AAAA\\]==\\]")
  message(FATAL_ERROR "Missing CMAKE_EP_GIT_REMOTE_UPDATE_STRATEGY")
endif()

if(NOT contents MATCHES "CMAKE_TLS_VERIFY \\[==\\[BBBB\\]==\\]")
  message(FATAL_ERROR "Missing CMAKE_TLS_VERIFY")
endif()

if(NOT contents MATCHES "CMAKE_TLS_CAINFO \\[==\\[CCCC\\]==\\]")
  message(FATAL_ERROR "Missing CMAKE_TLS_CAINFO")
endif()

if(NOT contents MATCHES "CMAKE_NETRC \\[==\\[DDDD\\]==\\]")
  message(FATAL_ERROR "Missing CMAKE_NETRC")
endif()

if(NOT contents MATCHES "CMAKE_NETRC_FILE \\[==\\[EEEE\\]==\\]")
  message(FATAL_ERROR "Missing CMAKE_NETRC_FILE")
endif()
