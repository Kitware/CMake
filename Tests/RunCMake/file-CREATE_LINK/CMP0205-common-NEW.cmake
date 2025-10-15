cmake_policy(SET CMP0205 NEW)
include("${CMAKE_CURRENT_LIST_DIR}/CMP0205-common.cmake")

if(NOT allFilesDst)
  message(SEND_ERROR "Destination directory is empty: '${allFilesDst}'")
endif()

if(NOT "${allFilesSrc}" STREQUAL "${allFilesDst}")
  message(SEND_ERROR "Source and destination directories are not equal")
  message(SEND_ERROR "Source files: '${allFilesSrc}'")
  message(SEND_ERROR "Destination files: '${allFilesDst}'")
endif()
