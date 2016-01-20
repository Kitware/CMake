find_package(PkgConfig REQUIRED)
pkg_check_modules(NCURSES QUIET ncurses)

if (NCURSES_FOUND)
  foreach (variable IN ITEMS PREFIX INCLUDEDIR LIBDIR)
    get_property("${variable}"
      CACHE     "NCURSES_${variable}"
      PROPERTY  TYPE
      SET)
    if (NOT ${variable})
      message(FATAL_ERROR "Failed to set cache entry for NCURSES_${variable}")
    endif ()
  endforeach ()
else ()
  message(STATUS "skipping test; ncurses not found")
endif ()
