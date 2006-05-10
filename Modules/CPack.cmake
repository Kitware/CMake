# Pick a configuration file
SET(cpack_input_file "${CMAKE_ROOT}/Templates/CPackConfig.cmake.in")
IF(EXISTS "${CMAKE_SOURCE_DIR}/CPackConfig.cmake.in")
  SET(cpack_input_file "${CMAKE_SOURCE_DIR}/CPackConfig.cmake.in")
ENDIF(EXISTS "${CMAKE_SOURCE_DIR}/CPackConfig.cmake.in")
SET(cpack_source_input_file "${CMAKE_ROOT}/Templates/CPackConfig.cmake.in")
IF(EXISTS "${CMAKE_SOURCE_DIR}/CPackSourceConfig.cmake.in")
  SET(cpack_source_input_file "${CMAKE_SOURCE_DIR}/CPackSourceConfig.cmake.in")
ENDIF(EXISTS "${CMAKE_SOURCE_DIR}/CPackSourceConfig.cmake.in")

# Macro for setting values if a user did not overwrite them
MACRO(cpack_set_if_not_set name value)
  IF(NOT DEFINED "${name}")
    SET(${name} "${value}")
  ENDIF(NOT DEFINED "${name}")
ENDMACRO(cpack_set_if_not_set)

# Macro to encode variables for the configuration file
MACRO(cpack_encode_variables)
  SET(_CPACK_OTHER_VARIABLES_)
  GET_CMAKE_PROPERTY(res VARIABLES)
  FOREACH(var ${res})
    IF("xxx${var}" MATCHES "xxxCPACK")
      SET(_CPACK_OTHER_VARIABLES_
        "${_CPACK_OTHER_VARIABLES_}\nSET(${var} \"${${var}}\")")
    ENDIF("xxx${var}" MATCHES "xxxCPACK")
  ENDFOREACH(var ${res})
ENDMACRO(cpack_encode_variables)


# Set the package name
cpack_set_if_not_set(CPACK_PACKAGE_NAME "${CMAKE_PROJECT_NAME}")
cpack_set_if_not_set(CPACK_PACKAGE_VERSION_MAJOR "0")
cpack_set_if_not_set(CPACK_PACKAGE_VERSION_MINOR "1")
cpack_set_if_not_set(CPACK_PACKAGE_VERSION_PATCH "1")
cpack_set_if_not_set(CPACK_PACKAGE_VERSION
  "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
cpack_set_if_not_set(CPACK_PACKAGE_VENDOR "Humanity")
cpack_set_if_not_set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
  "${CMAKE_PROJECT_NAME} built using CMake")

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

SET(__cpack_system_name ${CMAKE_SYSTEM_NAME})
IF(${__cpack_system_name} MATCHES Windows)
  IF(CMAKE_CL_64)
    SET(__cpack_system_name win64)
  ELSE(CMAKE_CL_64)
    SET(__cpack_system_name win32)
  ENDIF(CMAKE_CL_64)
ENDIF(${__cpack_system_name} MATCHES Windows)
cpack_set_if_not_set(CPACK_SYSTEM_NAME "${__cpack_system_name}")

# <project>-<major>.<minor>.<patch>-<release>-<platform>.<pkgtype>
cpack_set_if_not_set(CPACK_PACKAGE_FILE_NAME
  "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-${CPACK_SYSTEM_NAME}")
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
    SET(CPACK_SOURCE_GENERATOR "TGZ")
  ELSE(UNIX)
    SET(CPACK_GENERATOR "NSIS")
    SET(CPACK_SOURCE_GENERATOR "ZIP")
  ENDIF(UNIX)
ENDIF(NOT CPACK_GENERATOR)

# Set some other variables
cpack_set_if_not_set(CPACK_INSTALL_CMAKE_PROJECTS
  "${CMAKE_BINARY_DIR};${CMAKE_PROJECT_NAME};ALL;/")
cpack_set_if_not_set(CPACK_CMAKE_GENERATOR "${CMAKE_GENERATOR}")
cpack_set_if_not_set(CPACK_TOPLEVEL_TAG "${CPACK_SYSTEM_NAME}")

cpack_set_if_not_set(CPACK_NSIS_DISPLAY_NAME "@CPACK_PACKAGE_INSTALL_DIRECTORY@")

cpack_encode_variables()
CONFIGURE_FILE("${cpack_input_file}"
  "${CMAKE_BINARY_DIR}/CPackConfig.cmake" @ONLY IMMEDIATE)

# Generate source file
cpack_set_if_not_set(CPACK_SOURCE_INSTALLED_DIRECTORIES
  "${CMAKE_SOURCE_DIR};/")
cpack_set_if_not_set(CPACK_SOURCE_TOPLEVEL_TAG "${CPACK_SYSTEM_NAME}-Source")
cpack_set_if_not_set(CPACK_SOURCE_PACKAGE_FILE_NAME
  "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-Source")
cpack_set_if_not_set(CPACK_SOURCE_IGNORE_FILES
  "/CVS/;/\\\\\\\\.svn/;\\\\\\\\.swp$;\\\\\\\\.#;/#")
SET(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_SOURCE_INSTALL_CMAKE_PROJECTS}")
SET(CPACK_INSTALLED_DIRECTORIES "${CPACK_SOURCE_INSTALLED_DIRECTORIES}")
SET(CPACK_GENERATOR "${CPACK_SOURCE_GENERATOR}")
SET(CPACK_TOPLEVEL_TAG "${CPACK_SOURCE_TOPLEVEL_TAG}")
SET(CPACK_PACKAGE_FILE_NAME "${CPACK_SOURCE_PACKAGE_FILE_NAME}")
SET(CPACK_IGNORE_FILES "${CPACK_SOURCE_IGNORE_FILES}")
SET(CPACK_STRIP_FILES "${CPACK_SOURCE_STRIP_FILES}")

cpack_encode_variables()
CONFIGURE_FILE("${cpack_source_input_file}"
  "${CMAKE_BINARY_DIR}/CPackSourceConfig.cmake" @ONLY IMMEDIATE)
