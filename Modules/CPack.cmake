# Pick a configuration file
SET(cpack_input_file "${CMAKE_ROOT}/Templates/CPackConfig.cmake.in")
IF(EXISTS "${CMAKE_SOURCE_DIR}/CPackConfig.cmake.in")
  SET(cpack_input_file "${CMAKE_SOURCE_DIR}/CPackConfig.cmake.in")
ENDIF(EXISTS "${CMAKE_SOURCE_DIR}/CPackConfig.cmake.in")

# Macro for setting values if a user did not overwrite them
MACRO(cpack_set_if_not_set name value)
  IF(NOT DEFINED "${name}")
    SET(${name} "${value}")
  ENDIF(NOT DEFINED "${name}")
ENDMACRO(cpack_set_if_not_set)

# Set the package name
cpack_set_if_not_set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
cpack_set_if_not_set(CPACK_PACKAGE_VERSION_MAJOR "0")
cpack_set_if_not_set(CPACK_PACKAGE_VERSION_MINOR "1")
cpack_set_if_not_set(CPACK_PACKAGE_VERSION_PATCH "1")
cpack_set_if_not_set(CPACK_PACKAGE_VENDOR "Humanity")
cpack_set_if_not_set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
  "${PROJECT_NAME} built using CMake")
cpack_set_if_not_set(CPACK_PACKAGE_DESCRIPTION_FILE
  "${CMAKE_ROOT}/Templates/CPack.GenericDescription.txt")

# <project>-<major>.<minor>.<patch>-<release>-<platform>.<pkgtype>
cpack_set_if_not_set(CPACK_PACKAGE_FILE_NAME
  "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-${CMAKE_SYSTEM_NAME}")

IF(NOT EXISTS "${CPACK_PACKAGE_DESCRIPTION_FILE}")
  MESSAGE(SEND_ERROR "CPack package description file: \"${CPACK_PACKAGE_DESCRIPTION_FILE}\" could not be found.")
ENDIF(NOT EXISTS "${CPACK_PACKAGE_DESCRIPTION_FILE}")

# Pick a generator
IF(NOT CPACK_GENERATOR)
  IF(UNIX)
    IF(APPLE)
      SET(CPACK_GENERATOR "PackageMaker")
    ELSE(APPLE)
      SET(CPACK_GENERATOR "TGZ")
    ENDIF(APPLE)
  ELSE(UNIX)
    SET(CPACK_GENERATOR "NSIS")
  ENDIF(UNIX)
ENDIF(NOT CPACK_GENERATOR)

# Set some other variables
SET(CPACK_SOURCE_DIR "${CMAKE_SOURCE_DIR}")
SET(CPACK_BINARY_DIR "${CMAKE_BINARY_DIR}")

# Hack for Visual Studio support
# Search for system runtime libraries based on the platform.  This is
# not complete because it is used only for the release process by the
# developers.
IF(WIN32 AND NOT CYGWIN)
  STRING(REGEX REPLACE "\\\\" "/" SYSTEMROOT "$ENV{SYSTEMROOT}")
  FOREACH(lib
      "${SYSTEMROOT}/system32/mfc71.dll"
      "${SYSTEMROOT}/system32/msvcp71.dll"
      "${SYSTEMROOT}/system32/msvcr71.dll"
      )
    IF(EXISTS ${lib})
      SET(CMake_INSTALL_SYSTEM_RUNTIME_LIBS
        ${CMake_INSTALL_SYSTEM_RUNTIME_LIBS} ${lib})
    ENDIF(EXISTS ${lib})
  ENDFOREACH(lib)
ENDIF(WIN32 AND NOT CYGWIN)

# Include system runtime libraries in the installation if any are
# specified by CMake_INSTALL_SYSTEM_RUNTIME_LIBS.
IF(CMake_INSTALL_SYSTEM_RUNTIME_LIBS)
  IF(WIN32)
    INSTALL_PROGRAMS(/bin ${CMake_INSTALL_SYSTEM_RUNTIME_LIBS})
  ELSE(WIN32)
    INSTALL_PROGRAMS(/lib ${CMake_INSTALL_SYSTEM_RUNTIME_LIBS})
  ENDIF(WIN32)
ENDIF(CMake_INSTALL_SYSTEM_RUNTIME_LIBS)

CONFIGURE_FILE("${cpack_input_file}" "${CMAKE_BINARY_DIR}/CPackConfig.cmake" @ONLY IMMEDIATE)
