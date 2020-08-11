# Prepare environment to reuse bletch.pc
file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/pc-bletch/lib/pkgconfig" PC_PATH)
if(UNIX)
  string(REPLACE "\\ " " " PC_PATH "${PC_PATH}")
endif()
set(ENV{PKG_CONFIG_PATH} "${PC_PATH}")

find_package(PkgConfig REQUIRED)
pkg_check_modules(Bletch IMPORTED_TARGET REQUIRED bletch-framework)

if (Bletch_LDFLAGS_OTHER)
  message(SEND_ERROR "Bletch_LDFLAGS_OTHER should be empty, but is '${Bletch_LDFLAGS_OTHER}'")
endif ()

if (NOT Bletch_LINK_LIBRARIES STREQUAL "-framework foo;-framework bar;bletch;-framework baz")
  message(SEND_ERROR "Bletch_LINK_LIBRARIES has wrong value '${Bletch_LINK_LIBRARIES}'")
endif ()

foreach (prop IN ITEMS INTERFACE_INCLUDE_DIRECTORIES INTERFACE_LINK_OPTIONS INTERFACE_COMPILE_OPTIONS)
  get_target_property(prop_value PkgConfig::Bletch ${prop})
  if (prop_value)
    message(SEND_ERROR "target property ${prop} should not be set, but is '${prop_value}'")
  endif ()
endforeach ()

get_target_property(prop_value PkgConfig::Bletch INTERFACE_LINK_LIBRARIES)
if (NOT prop_value STREQUAL Bletch_LINK_LIBRARIES)
  message(SEND_ERROR "target property INTERFACE_LINK_LIBRARIES has wrong value '${prop_value}'")
endif ()
