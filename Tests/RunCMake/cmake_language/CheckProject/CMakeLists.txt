
cmake_language (CALL cmake_minimum_required VERSION 3.17...3.18)

cmake_language (CALL project CheckProject VERSION 1.2.3 LANGUAGES C)

if (NOT PROJECT_NAME STREQUAL "CheckProject")
  message (SEND_ERROR "error on project() usage.")
endif()

if (NOT CheckProject_VERSION VERSION_EQUAL "1.2.3")
  message (SEND_ERROR "error on project() usage.")
endif()

get_property (languages GLOBAL PROPERTY ENABLED_LANGUAGES)
if (NOT "C" IN_LIST languages)
  message (SEND_ERROR "error on project() usage.")
endif()

add_library (lib SHARED lib.c)
