# CMP0207 is unset
include("${CMAKE_CURRENT_LIST_DIR}/CMP0207-common.cmake")

install(CODE [[
  if(NOT results_old)
    message(SEND_ERROR "Old dependencies are empty: `${results_old}`")
  endif()

  if(results_new)
    message(SEND_ERROR "New dependencies are not empty: `${results_new}`")
  endif()
  ]])
