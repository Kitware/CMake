project(InterfaceBuildTargets)

add_library(testlib testlib.cxx)
set_property(TARGET testlib PROPERTY PREFIX "")
if(CMAKE_GENERATOR MATCHES "Borland|Watcom")
  # These librarians add the .lib suffix anyway.
  set_property(TARGET testlib PROPERTY SUFFIX ".lib")
else()
  set_property(TARGET testlib PROPERTY SUFFIX "")
endif()

add_library(iface INTERFACE)
target_link_libraries(iface INTERFACE testlib)
