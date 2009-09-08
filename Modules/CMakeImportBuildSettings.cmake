
# This module is purposely no longer documented.  It does nothing useful.

# This macro used to load build settings from another project that
# stored settings using the CMAKE_EXPORT_BUILD_SETTINGS macro.
MACRO(CMAKE_IMPORT_BUILD_SETTINGS SETTINGS_FILE)
  IF(${SETTINGS_FILE} MATCHES ".+")
  ELSE(${SETTINGS_FILE} MATCHES ".+")
    MESSAGE(SEND_ERROR "CMAKE_IMPORT_BUILD_SETTINGS called with no argument.")
  ENDIF(${SETTINGS_FILE} MATCHES ".+")
ENDMACRO(CMAKE_IMPORT_BUILD_SETTINGS)
