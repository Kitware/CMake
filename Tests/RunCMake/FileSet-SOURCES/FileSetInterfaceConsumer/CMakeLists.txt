foreach (id IN ITEMS 0 1 2)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/inc${id}/consumer_id.h"
    "#define INC_ID ${id}\n")
endforeach()
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/owner_local_inc/owner_local.h"
  "#define OWNER_LOCAL_INC\n")

# Visible only in this directory, so it must be looked up here rather than in
# the directory of the consumers.
add_library(owner_local INTERFACE IMPORTED)
set_property(TARGET owner_local PROPERTY LOCAL_DEF OWNER_LOCAL_DEF)
set_property(TARGET owner_local PROPERTY LOCAL_OPT -DOWNER_LOCAL_OPT)
set_property(TARGET owner_local PROPERTY LOCAL_INC owner_local_inc)

add_library(iface INTERFACE)
target_sources(iface INTERFACE FILE_SET a TYPE SOURCES FILES consumer.c)
set_property(TARGET iface PROPERTY FS_ID 0)

set_property(FILE_SET a TARGET iface PROPERTY INTERFACE_COMPILE_DEFINITIONS
  DEF_ID=$<TARGET_PROPERTY:FS_ID>
  $<TARGET_PROPERTY:owner_local,LOCAL_DEF>)
set_property(FILE_SET a TARGET iface PROPERTY INTERFACE_COMPILE_OPTIONS
  -DOPT_ID=$<TARGET_PROPERTY:FS_ID>
  $<TARGET_PROPERTY:owner_local,LOCAL_OPT>)
set_property(FILE_SET a TARGET iface PROPERTY INTERFACE_INCLUDE_DIRECTORIES
  "${CMAKE_CURRENT_BINARY_DIR}/inc$<TARGET_PROPERTY:FS_ID>"
  "${CMAKE_CURRENT_BINARY_DIR}/$<TARGET_PROPERTY:owner_local,LOCAL_INC>")
