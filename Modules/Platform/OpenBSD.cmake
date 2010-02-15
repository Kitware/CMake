INCLUDE(Platform/NetBSD)

# On OpenBSD, the compile time linker does not share it's configuration with
# the runtime linker.  This will extract the library search paths from the
# system's ld.so.hints file which will allow CMake to set the appropriate
# -rpath-link flags
IF(NOT CMAKE_PLATFORM_RUNTIME_PATH)
  EXECUTE_PROCESS(COMMAND /sbin/ldconfig -r
                  OUTPUT_VARIABLE LDCONFIG_HINTS
                  ERROR_QUIET)
  STRING(REGEX REPLACE ".*search\\ directories:\\ ([^\n]*).*" "\\1"
         LDCONFIG_HINTS "${LDCONFIG_HINTS}")
  STRING(REGEX REPLACE ":" ";"
         CMAKE_PLATFORM_RUNTIME_PATH
         "${LDCONFIG_HINTS}")
ENDIF()

SET_PROPERTY(GLOBAL PROPERTY FIND_LIBRARY_USE_OPENBSD_VERSIONING 1)
