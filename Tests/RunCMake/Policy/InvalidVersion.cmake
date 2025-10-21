if(NOT version)
  message(SEND_ERROR "Version string is empty: ${version}")
endif()

cmake_policy(VERSION ${version})
