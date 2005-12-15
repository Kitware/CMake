# - export build settings from a project.
#  CMAKE_EXPORT_BUILD_SETTINGS(SETTINGS_FILE)
# macro defined to export the build settings for use by another project.
#  SETTINGS_FILE - the file into which the settings are to be stored.
MACRO(CMAKE_EXPORT_BUILD_SETTINGS SETTINGS_FILE)
  IF(${SETTINGS_FILE} MATCHES ".+")
    CONFIGURE_FILE(${CMAKE_ROOT}/Modules/CMakeBuildSettings.cmake.in
                   ${SETTINGS_FILE} @ONLY IMMEDIATE)
  ELSE(${SETTINGS_FILE} MATCHES ".+")
    MESSAGE(SEND_ERROR "CMAKE_EXPORT_BUILD_SETTINGS called with no argument.")
  ENDIF(${SETTINGS_FILE} MATCHES ".+")
ENDMACRO(CMAKE_EXPORT_BUILD_SETTINGS)
