# CMP0205 is unset
include("${CMAKE_CURRENT_LIST_DIR}/CMP0205-common.cmake")

if(allFilesDst)
  message(SEND_ERROR "Directory is not empty: '${allFilesDst}'")
endif()
