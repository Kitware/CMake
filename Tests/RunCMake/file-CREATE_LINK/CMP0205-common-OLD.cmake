cmake_policy(SET CMP0205 OLD)
include("${CMAKE_CURRENT_LIST_DIR}/CMP0205-common.cmake")

if(allFilesDst)
  message(SEND_ERROR "Directory is not empty: '${allFilesDst}'")
endif()
