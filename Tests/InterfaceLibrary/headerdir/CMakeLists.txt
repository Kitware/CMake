
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

add_library(headeriface INTERFACE)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/iface_header_builddir.h"
  "#define IFACE_HEADER_BUILDDIR\n"
)
