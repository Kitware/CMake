cmake_policy(SET CMP0207 NEW)

include("${CMAKE_CURRENT_LIST_DIR}/CMP0207-common.cmake")

install(CODE [[
  if(results_old)
    message(SEND_ERROR "Old dependencies are not empty: `${results_old}`")
  endif()

  if(NOT results_new)
    message(SEND_ERROR "New dependencies are empty: `${results_new}`")
  endif()
  ]])
