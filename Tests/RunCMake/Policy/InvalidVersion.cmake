if(NOT version)
  message(SEND_ERROR "Vesrion string is empty: ${version}")
endif()

cmake_policy(VERSION ${version})
