
add_library(headeronly INTERFACE)
set_property(TARGET headeronly PROPERTY INTERFACE_INCLUDE_DIRECTORIES
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/headeronly>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/headeronly>"
)
set_property(TARGET headeronly PROPERTY INTERFACE_COMPILE_DEFINITIONS "HEADERONLY_DEFINE")

include(GenerateExportHeader)
add_library(sharedlib SHARED sharedlib.cpp)
generate_export_header(sharedlib)
set_property(TARGET sharedlib PROPERTY INCLUDE_DIRECTORIES
  "${CMAKE_CURRENT_SOURCE_DIR}/sharedlib"
  "${CMAKE_CURRENT_BINARY_DIR}"
)
set_property(TARGET sharedlib PROPERTY INTERFACE_INCLUDE_DIRECTORIES
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/sharedlib;${CMAKE_CURRENT_BINARY_DIR}>"
  "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/sharedlib>"
)

set_property(TARGET sharedlib PROPERTY INTERFACE_COMPILE_DEFINITIONS "SHAREDLIB_DEFINE")

add_library(sharediface INTERFACE)
target_link_libraries(sharediface INTERFACE sharedlib)

export(TARGETS sharediface sharedlib headeronly
  NAMESPACE bld_
  FILE ../ExportInterfaceBuildTree.cmake
)

install(TARGETS headeronly sharediface sharedlib
  EXPORT expInterface
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib NAMELINK_SKIP
  ARCHIVE DESTINATION lib
  FRAMEWORK DESTINATION Frameworks
  BUNDLE DESTINATION Applications
)
install(FILES
  headeronly/headeronly.h
  DESTINATION include/headeronly
)
install(FILES
  sharedlib/sharedlib.h
  "${CMAKE_CURRENT_BINARY_DIR}/sharedlib_export.h"
  DESTINATION include/sharedlib
)

install(EXPORT expInterface NAMESPACE exp_ DESTINATION lib/exp)
