# CMP0205 is unset
include("${CMAKE_CURRENT_LIST_DIR}/CMP0205-common.cmake")

# We only really care about when COPY_ON_ERROR was actually executed, but we'll
# test both cases for posterity.

if(NOT madeSymlink)
  if(allFilesDst)
    message(SEND_ERROR "Directory is not empty: '${allFilesDst}'")
  endif()
else()
  if(NOT allFilesDst)
    message(SEND_ERROR "Destination directory is empty: '${allFilesDst}'")
  endif()
endif()
