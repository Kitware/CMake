find_package(PkgConfig REQUIRED)

set(fakePkgDir ${CMAKE_CURRENT_BINARY_DIR}/pc-shadow)
set(ENV{PKG_CONFIG_PATH} "${fakePkgDir}/lib/pkgconfig")

file(WRITE ${fakePkgDir}/lib/pkgconfig/cmakeinternalshadow.pc
"Name: CMakeInternalShadow
Description: Dummy package for FindPkgConfig shadowing test
Version: 1.0.0
Cflags: -I${fakePkgDir}/include -DSHADOW
Libs: -L${fakePkgDir}/lib -lshadow -Wl,--as-needed
")

set(vars
  INCLUDE_DIRS CFLAGS CFLAGS_OTHER
  LIBRARIES LIBRARY_DIRS LDFLAGS LDFLAGS_OTHER
  STATIC_INCLUDE_DIRS STATIC_CFLAGS STATIC_CFLAGS_OTHER
  STATIC_LIBRARIES STATIC_LIBRARY_DIRS STATIC_LDFLAGS STATIC_LDFLAGS_OTHER
)

# Reference results, with no pre-existing normal variables
pkg_check_modules(REF REQUIRED cmakeinternalshadow)
foreach(v IN LISTS vars)
  if(REF_${v} STREQUAL "")
    message(SEND_ERROR "REF_${v} is unexpectedly empty")
  endif()
endforeach()

# Normal variables set by project code before the call must not hide
# the results of pkg_check_modules
function(check_shadow)
  foreach(v IN LISTS vars)
    set(SHADOW_${v} "")
  endforeach()

  pkg_check_modules(SHADOW REQUIRED cmakeinternalshadow)
  foreach(v IN LISTS vars)
    if(NOT "${SHADOW_${v}}" STREQUAL "${REF_${v}}")
      message(SEND_ERROR "SHADOW_${v} is shadowed by a normal variable:\n"
        "expected -->${REF_${v}}<--\nreceived -->${SHADOW_${v}}<--")
    endif()
  endforeach()
endfunction()

check_shadow()
