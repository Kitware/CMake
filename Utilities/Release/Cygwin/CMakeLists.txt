FILE(GLOB INSTALLED_CURSES /usr/bin/cygncurses*.dll)
SET(MAX 0)
FOREACH(f ${INSTALLED_CURSES})
  IF(NOT "${f}" MATCHES "\\+")
    STRING(REGEX REPLACE ".*([0-9]).dll" "\\1" NUMBER "${f}")
    IF(NUMBER GREATER MAX)
      SET(MAX ${NUMBER}) 
    ENDIF(NUMBER GREATER MAX)
  ENDIF(NOT "${f}" MATCHES "\\+")
ENDFOREACH(f)
STRING(REGEX REPLACE "/usr/bin/" "\\1" NUMBER "${f}")
SET(CMAKE_NCURSES_VERSION "libncurses${MAX}")
MESSAGE(STATUS "Using curses version: libncurses${MAX}")
CONFIGURE_FILE("${CMake_SOURCE_DIR}/Utilities/Release/Cygwin/cygwin-setup.hint.in"
  "${CMake_BINARY_DIR}/setup.hint")
CONFIGURE_FILE("${CMake_SOURCE_DIR}/Utilities/Release/Cygwin/README.cygwin.in"
  "${CMake_BINARY_DIR}/Docs/@CPACK_PACKAGE_FILE_NAME@-@CPACK_CYGWIN_PATCH_NUMBER@.README")
INSTALL_FILES(/share/doc/Cygwin FILES
  ${CMake_BINARY_DIR}/Docs/@CPACK_PACKAGE_FILE_NAME@-@CPACK_CYGWIN_PATCH_NUMBER@.README
  )
CONFIGURE_FILE("${CMake_SOURCE_DIR}/Utilities/Release/Cygwin/cygwin-package.sh.in"
  ${CPACK_CYGWIN_BUILD_SCRIPT})
CONFIGURE_FILE("${CMake_SOURCE_DIR}/Utilities/Release/Cygwin/cygwin-patch.diff.in"
  ${CPACK_CYGWIN_PATCH_FILE})

