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
cpack_set_if_not_set(CPACK_PACKAGE_VERSION
  "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
cpack_set_if_not_set(CPACK_PACKAGE_VENDOR "Humanity")
cpack_set_if_not_set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
  "${PROJECT_NAME} built using CMake")

cpack_set_if_not_set(CPACK_PACKAGE_DESCRIPTION_FILE
  "${CMAKE_ROOT}/Templates/CPack.GenericDescription.txt")
cpack_set_if_not_set(CPACK_RESOURCE_FILE_LICENSE
  "${CMAKE_ROOT}/Templates/CPack.GenericLicense.txt")
cpack_set_if_not_set(CPACK_RESOURCE_FILE_README
  "${CMAKE_ROOT}/Templates/CPack.GenericDescription.txt")
cpack_set_if_not_set(CPACK_RESOURCE_FILE_WELCOME
  "${CMAKE_ROOT}/Templates/CPack.GenericWelcome.txt")

IF(CPACK_NSIS_MODIFY_PATH)
  SET(CPACK_NSIS_MODIFY_PATH ON)
ENDIF(CPACK_NSIS_MODIFY_PATH)

# <project>-<major>.<minor>.<patch>-<release>-<platform>.<pkgtype>
cpack_set_if_not_set(CPACK_PACKAGE_FILE_NAME
  "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-${CMAKE_SYSTEM_NAME}")
cpack_set_if_not_set(CPACK_PACKAGE_INSTALL_DIRECTORY
  "${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION}")
cpack_set_if_not_set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY
  "${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION}")

MACRO(cpack_check_file_exists file description)
IF(NOT EXISTS "${file}")
  MESSAGE(SEND_ERROR "CPack ${description} file: \"${file}\" could not be found.")
ENDIF(NOT EXISTS "${file}")
ENDMACRO(cpack_check_file_exists)
cpack_check_file_exists("${CPACK_PACKAGE_DESCRIPTION_FILE}" "package description")
cpack_check_file_exists("${CPACK_RESOURCE_FILE_LICENSE}"    "license resource")
cpack_check_file_exists("${CPACK_RESOURCE_FILE_README}"     "readme resource")
cpack_check_file_exists("${CPACK_RESOURCE_FILE_WELCOME}"    "welcome resource")

# Pick a generator
IF(NOT CPACK_GENERATOR)
  IF(UNIX)
    IF(APPLE)
      SET(CPACK_GENERATOR "PackageMaker")
    ELSE(APPLE)
      SET(CPACK_GENERATOR "STGZ")
    ENDIF(APPLE)
  ELSE(UNIX)
    SET(CPACK_GENERATOR "NSIS")
  ENDIF(UNIX)
ENDIF(NOT CPACK_GENERATOR)

# Set some other variables
SET(CPACK_BINARY_DIR "${CMAKE_BINARY_DIR}")

SET(_CPACK_UNUSED_VARIABLES_)
GET_CMAKE_PROPERTY(res VARIABLES)
FOREACH(var ${res})
  IF("xxx${var}" MATCHES "xxxCPACK")
    SET(_CPACK_OTHER_VARIABLES_
      "${_CPACK_OTHER_VARIABLES_}\nSET(${var} \"${${var}}\")")
  ENDIF("xxx${var}" MATCHES "xxxCPACK")
ENDFOREACH(var ${res})

CONFIGURE_FILE("${cpack_input_file}" "${CMAKE_BINARY_DIR}/CPackConfig.cmake" @ONLY IMMEDIATE)
