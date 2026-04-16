# Use COPY_ON_ERROR to handle the case where the source and destination
# directory are on different devices and empty.
file(CREATE_LINK
  ${CMAKE_CURRENT_LIST_DIR}/CMP0205 ${CMAKE_CURRENT_BINARY_DIR}/CMP0205-${link_name}
  ${maybe_SYMBOLIC}
  RESULT result
  COPY_ON_ERROR
  )
if(NOT result STREQUAL "0")
  message(SEND_ERROR "COPY_ON_ERROR failed: '${result}'")
endif()

# When CMP0205 is NEW, we must verify after running command this again that:
# * on systems which support directory symlinks, the source directory to which
#   the newly-created link points is not deleted, only the symlink itself.
# * on systems which do not support directory symlinks, the destination
#   directory which was created via COPY_ON_ERROR is appropriately deleted
#   at the beginning of executing this command, before creating the new link
#   (and copying instead, again).
cmake_policy(GET CMP0205 _cmp0205)
if("${maybe_SYMBOLIC}" STREQUAL "SYMBOLIC" AND "${_cmp0205}" STREQUAL "NEW")
  file(CREATE_LINK
    ${CMAKE_CURRENT_LIST_DIR}/CMP0205 ${CMAKE_CURRENT_BINARY_DIR}/CMP0205-${link_name}
    ${maybe_SYMBOLIC}
    RESULT result
    COPY_ON_ERROR
    )
  if(NOT result STREQUAL "0")
    message(SEND_ERROR "COPY_ON_ERROR failed: '${result}'")
  endif()
endif()

set(madeSymlink OFF)
if(IS_SYMLINK ${CMAKE_CURRENT_BINARY_DIR}/CMP0205-${link_name})
  set(madeSymlink ON)
endif()

file(GLOB_RECURSE allFilesSrc LIST_DIRECTORIES true RELATIVE "${CMAKE_CURRENT_LIST_DIR}/CMP0205" "${CMAKE_CURRENT_LIST_DIR}/CMP0205/*")
file(GLOB_RECURSE allFilesDst LIST_DIRECTORIES true RELATIVE "${CMAKE_CURRENT_BINARY_DIR}/CMP0205-${link_name}" "${CMAKE_CURRENT_BINARY_DIR}/CMP0205-${link_name}/*")
