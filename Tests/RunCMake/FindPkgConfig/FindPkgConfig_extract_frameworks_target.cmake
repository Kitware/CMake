cmake_minimum_required(VERSION 3.17)

# Prepare environment to reuse bletch.pc
file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/pc-bletch/lib/pkgconfig" PC_PATH)
if(UNIX)
  string(REPLACE "\\ " " " PC_PATH "${PC_PATH}")
endif()
set(ENV{PKG_CONFIG_PATH} "${PC_PATH}")

find_package(PkgConfig REQUIRED)

# to test multiple variations, we must pick unique prefix names (same-named targets are cached for re-use)
set(prefix_uniquifiers 0 1)
# whether to apply STATIC_TARGET argument
set(static_target_args "" STATIC_TARGET)
# whether target properties are populated from the unqualified (i.e. shared library) series of vars, or the STATIC_ series of vars
set(target_var_qualifiers "" STATIC_)
foreach (prefix_uniquifier static_target_arg target_var_qualifier IN ZIP_LISTS prefix_uniquifiers static_target_args target_var_qualifiers)
  set(prefix "Bletch${prefix_uniquifier}")
  set(tgt "PkgConfig::${prefix}")
  pkg_check_modules(${prefix} IMPORTED_TARGET REQUIRED ${static_target_arg} bletch-framework)
  foreach (prop IN ITEMS INTERFACE_INCLUDE_DIRECTORIES INTERFACE_LINK_OPTIONS INTERFACE_COMPILE_OPTIONS)
    get_target_property(prop_value ${tgt} ${prop})
    if (prop_value)
      message(SEND_ERROR "target property ${prop} should not be set, but is '${prop_value}'")
    endif ()
  endforeach ()

  # there is 1 target yet 2 series of variables.
  # if STATIC_TARGET is set, then the target will follow the STATIC_ qualified series of variables
  # (otherwise will follow the unqualified series of variables).
  get_target_property(prop_value ${tgt} INTERFACE_LINK_LIBRARIES)
  if (NOT prop_value STREQUAL ${prefix}_${target_var_qualifier}LINK_LIBRARIES)
    message(SEND_ERROR "target property INTERFACE_LINK_LIBRARIES has wrong value '${prop_value}'")
  endif ()

  foreach (var_qualifier IN ITEMS "" STATIC_)
    set (ldflags_var ${prefix}_${var_qualifier}LDFLAGS_OTHER)
    if (${ldflags_var})
      message(SEND_ERROR "${ldflags_var} should be empty, but is '${${ldflags_var}}'")
    endif ()

    set (linklibs_var ${prefix}_${var_qualifier}LINK_LIBRARIES)
    if (NOT ${linklibs_var} STREQUAL "-framework foo;-framework bar;bletch;-framework baz")
      message(SEND_ERROR "${linklibs_var} has wrong value '${${linklibs_var}}'")
    endif ()
  endforeach()
endforeach()
