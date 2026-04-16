cmake_policy(SET CMP0205 NEW)
include("${CMAKE_CURRENT_LIST_DIR}/CMP0205-common.cmake")

# Note that unlike the tests for CMP0205 OLD and WARN, the resulting files
# in the source and destination should be the same here regardless of whether
# COPY_ON_ERROR was actually executed (i.e, whether through the link, or actual
# files).

if(NOT allFilesSrc)
  message(SEND_ERROR "Source directory is empty: '${allFilesSrc}'")
endif()

if(NOT allFilesDst)
  message(SEND_ERROR "Destination directory is empty: '${allFilesDst}'")
endif()

if(NOT "${allFilesSrc}" STREQUAL "${allFilesDst}")
  message(SEND_ERROR "Source and destination directories are not equal")
  message(SEND_ERROR "Source files: '${allFilesSrc}'")
  message(SEND_ERROR "Destination files: '${allFilesDst}'")
endif()
