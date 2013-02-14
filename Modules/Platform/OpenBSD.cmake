include(Platform/NetBSD)

# On OpenBSD, the compile time linker does not share it's configuration with
# the runtime linker.  This will extract the library search paths from the
# system's ld.so.hints file which will allow CMake to set the appropriate
# -rpath-link flags
if(NOT CMAKE_PLATFORM_RUNTIME_PATH)
  execute_process(COMMAND /sbin/ldconfig -r
                  OUTPUT_VARIABLE LDCONFIG_HINTS
                  ERROR_QUIET)
  string(REGEX REPLACE ".*search\\ directories:\\ ([^\n]*).*" "\\1"
         LDCONFIG_HINTS "${LDCONFIG_HINTS}")
  string(REGEX REPLACE ":" ";"
         CMAKE_PLATFORM_RUNTIME_PATH
         "${LDCONFIG_HINTS}")
endif()

set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_OPENBSD_VERSIONING 1)

# OpenBSD policy requires that shared libraries be installed without
# executable permission.
set(CMAKE_INSTALL_SO_NO_EXE 1)
