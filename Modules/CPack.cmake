# Default output files will be CPackConfig.cmake and CPackSourceConfig.cmake.
# This can be overwritten with CPACK_OUTPUT_CONFIG_FILE and
# CPACK_SOURCE_OUTPUT_CONFIG_FILE.

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
# find any varable that stars with CPACK and create a variable
# _CPACK_OTHER_VARIABLES_ that contains SET commands for
# each cpack variable.  _CPACK_OTHER_VARIABLES_ is then
# used as an @ replacment in configure_file for the CPackConfig.
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

cpack_set_if_not_set(CPACK_MODULE_PATH "${CMAKE_MODULE_PATH}")

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
  "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}")
cpack_set_if_not_set(CPACK_PACKAGE_INSTALL_DIRECTORY
  "${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION}")
cpack_set_if_not_set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY
  "${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION}")
cpack_set_if_not_set(CPACK_PACKAGE_DEFAULT_LOCATION "/")
cpack_set_if_not_set(CPACK_PACKAGE_RELOCATABLE "true")

# always force to exactly "true" or "false" for CPack.Info.plist.in:
if(CPACK_PACKAGE_RELOCATABLE)
  set(CPACK_PACKAGE_RELOCATABLE "true")
else(CPACK_PACKAGE_RELOCATABLE)
  set(CPACK_PACKAGE_RELOCATABLE "false")
endif(CPACK_PACKAGE_RELOCATABLE)

macro(cpack_check_file_exists file description)
  if(NOT EXISTS "${file}")
    message(SEND_ERROR "CPack ${description} file: \"${file}\" could not be found.")
  endif(NOT EXISTS "${file}")
endmacro(cpack_check_file_exists)

cpack_check_file_exists("${CPACK_PACKAGE_DESCRIPTION_FILE}" "package description")
cpack_check_file_exists("${CPACK_RESOURCE_FILE_LICENSE}"    "license resource")
cpack_check_file_exists("${CPACK_RESOURCE_FILE_README}"     "readme resource")
cpack_check_file_exists("${CPACK_RESOURCE_FILE_WELCOME}"    "welcome resource")

macro(cpack_optional_append _list _cond _item)
  if(${_cond})
    set(${_list} ${${_list}} ${_item})
  endif(${_cond})
endmacro(cpack_optional_append _list _cond _item)

# Provide options to choose generators
# we might check here if the required tools for the generates exist
# and set the defaults according to the results
if(NOT CPACK_GENERATOR)
  if(UNIX)
    if(CYGWIN)
      option(CPACK_BINARY_CYGWIN "Enable to build Cygwin binary packages" ON)
    else(CYGWIN)
      if(APPLE)
        option(CPACK_BINARY_PACKAGEMAKER "Enable to build PackageMaker packages" ON)
        option(CPACK_BINARY_OSXX11       "Enable to build OSX X11 packages"      OFF)
      else(APPLE)
        option(CPACK_BINARY_TZ  "Enable to build TZ packages"     ON)
      endif(APPLE)
      option(CPACK_BINARY_STGZ "Enable to build STGZ packages"    ON)
      option(CPACK_BINARY_TGZ  "Enable to build TGZ packages"     ON)
      option(CPACK_BINARY_TBZ2 "Enable to build TBZ2 packages"    ON)
      option(CPACK_BINARY_DEB  "Enable to build Debian packages"  OFF)
      option(CPACK_BINARY_RPM  "Enable to build RPM packages"     OFF)
      option(CPACK_BINARY_NSIS "Enable to build NSIS packages"    OFF)
    endif(CYGWIN)
  else(UNIX)
    option(CPACK_BINARY_NSIS "Enable to build NSIS packages" ON)
    option(CPACK_BINARY_ZIP  "Enable to build ZIP packages" ON)
  endif(UNIX)
  
  cpack_optional_append(CPACK_GENERATOR  CPACK_BINARY_PACKAGEMAKER PackageMaker)
  cpack_optional_append(CPACK_GENERATOR  CPACK_BINARY_OSXX11       OSXX11)
  cpack_optional_append(CPACK_GENERATOR  CPACK_BINARY_CYGWIN       CygwinBinary)
  cpack_optional_append(CPACK_GENERATOR  CPACK_BINARY_DEB          DEB)
  cpack_optional_append(CPACK_GENERATOR  CPACK_BINARY_RPM          RPM)
  cpack_optional_append(CPACK_GENERATOR  CPACK_BINARY_NSIS         NSIS)
  cpack_optional_append(CPACK_GENERATOR  CPACK_BINARY_STGZ         STGZ)
  cpack_optional_append(CPACK_GENERATOR  CPACK_BINARY_TGZ          TGZ)
  cpack_optional_append(CPACK_GENERATOR  CPACK_BINARY_TBZ2         TBZ2)
  cpack_optional_append(CPACK_GENERATOR  CPACK_BINARY_TZ           TZ)
  cpack_optional_append(CPACK_GENERATOR  CPACK_BINARY_ZIP          ZIP)
  
endif(NOT CPACK_GENERATOR)

# Provide options to choose source generators
if(NOT CPACK_SOURCE_GENERATOR)
  if(UNIX)
    if(CYGWIN)
      option(CPACK_SOURCE_CYGWIN "Enable to build Cygwin source packages" ON)
    else(CYGWIN)
      option(CPACK_SOURCE_TBZ2 "Enable to build TBZ2 source packages" ON)
      option(CPACK_SOURCE_TGZ  "Enable to build TGZ source packages"  ON)
      option(CPACK_SOURCE_TZ   "Enable to build TZ source packages"   ON)
      option(CPACK_SOURCE_ZIP  "Enable to build ZIP source packages"  OFF)
    endif(CYGWIN)
  else(UNIX)
    option(CPACK_SOURCE_ZIP "Enable to build ZIP source packages" ON)
  endif(UNIX)

  cpack_optional_append(CPACK_SOURCE_GENERATOR  CPACK_SOURCE_CYGWIN  CygwinSource)
  cpack_optional_append(CPACK_SOURCE_GENERATOR  CPACK_SOURCE_TGZ     TGZ)
  cpack_optional_append(CPACK_SOURCE_GENERATOR  CPACK_SOURCE_TBZ2    TBZ2)
  cpack_optional_append(CPACK_SOURCE_GENERATOR  CPACK_SOURCE_TZ      TZ)
  cpack_optional_append(CPACK_SOURCE_GENERATOR  CPACK_SOURCE_ZIP     ZIP)
endif(NOT CPACK_SOURCE_GENERATOR)

# mark the above options as advanced
mark_as_advanced(CPACK_BINARY_CYGWIN CPACK_BINARY_PACKAGEMAKER CPACK_BINARY_OSXX11
                 CPACK_BINARY_STGZ   CPACK_BINARY_TGZ          CPACK_BINARY_TBZ2 
                 CPACK_BINARY_DEB    CPACK_BINARY_RPM          CPACK_BINARY_TZ     
                 CPACK_BINARY_NSIS CPACK_BINARY_ZIP 
                 CPACK_SOURCE_CYGWIN CPACK_SOURCE_TBZ2 CPACK_SOURCE_TGZ 
                 CPACK_SOURCE_TZ CPACK_SOURCE_ZIP)

# Set some other variables
cpack_set_if_not_set(CPACK_INSTALL_CMAKE_PROJECTS
  "${CMAKE_BINARY_DIR};${CMAKE_PROJECT_NAME};ALL;/")
cpack_set_if_not_set(CPACK_CMAKE_GENERATOR "${CMAKE_GENERATOR}")
cpack_set_if_not_set(CPACK_TOPLEVEL_TAG "${CPACK_SYSTEM_NAME}")

cpack_set_if_not_set(CPACK_NSIS_DISPLAY_NAME "@CPACK_PACKAGE_INSTALL_DIRECTORY@")

cpack_set_if_not_set(CPACK_OUTPUT_CONFIG_FILE
  "${CMAKE_BINARY_DIR}/CPackConfig.cmake")

cpack_set_if_not_set(CPACK_SOURCE_OUTPUT_CONFIG_FILE
  "${CMAKE_BINARY_DIR}/CPackSourceConfig.cmake")

cpack_set_if_not_set(CPACK_SET_DESTDIR OFF)
cpack_set_if_not_set(CPACK_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

cpack_set_if_not_set(CPACK_NSIS_INSTALLER_ICON_CODE "")
cpack_set_if_not_set(CPACK_NSIS_INSTALLER_MUI_ICON_CODE "")

cpack_encode_variables()
configure_file("${cpack_input_file}" "${CPACK_OUTPUT_CONFIG_FILE}" @ONLY IMMEDIATE)

# Generate source file
cpack_set_if_not_set(CPACK_SOURCE_INSTALLED_DIRECTORIES
  "${CMAKE_SOURCE_DIR};/")
cpack_set_if_not_set(CPACK_SOURCE_TOPLEVEL_TAG "${CPACK_SYSTEM_NAME}-Source")
cpack_set_if_not_set(CPACK_SOURCE_PACKAGE_FILE_NAME
  "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-Source")
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
configure_file("${cpack_source_input_file}"
  "${CPACK_SOURCE_OUTPUT_CONFIG_FILE}" @ONLY IMMEDIATE)
