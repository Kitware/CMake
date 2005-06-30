#########################################################################
# Setup release scripts.

# Search for system runtime libraries based on the platform.  This is
# not complete because it is used only for the release process by the
# developers.
IF(WIN32)
  STRING(REGEX REPLACE "\\\\" "/" SYSTEMROOT "$ENV{SYSTEMROOT}")
  FOREACH(lib
      "${SYSTEMROOT}/system32/msvcp71.dll"
      "${SYSTEMROOT}/system32/msvcr71.dll"
      )
    IF(EXISTS ${lib})
      SET(CMake_INSTALL_SYSTEM_RUNTIME_LIBS
        ${CMake_INSTALL_SYSTEM_RUNTIME_LIBS} ${lib})
    ENDIF(EXISTS ${lib})
  ENDFOREACH(lib)
ENDIF(WIN32)

# Include system runtime libraries in the installation if any are
# specified by CMake_INSTALL_SYSTEM_RUNTIME_LIBS.
IF(CMake_INSTALL_SYSTEM_RUNTIME_LIBS)
  IF(WIN32)
    INSTALL_PROGRAMS(${CMake_INSTALL_BIN_DIR} 
      ${CMake_INSTALL_SYSTEM_RUNTIME_LIBS})
  ELSE(WIN32)
    INSTALL_PROGRAMS(${CMake_INSTALL_LIB_DIR} 
      ${CMake_INSTALL_SYSTEM_RUNTIME_LIBS})
  ENDIF(WIN32)
ENDIF(CMake_INSTALL_SYSTEM_RUNTIME_LIBS)

IF(WIN32)
  FIND_PROGRAM(NSIS_MAKENSIS NAMES makensis
    PATHS [HKEY_LOCAL_MACHINE\\SOFTWARE\\NSIS]
    DOC "Where is makensis.exe located"
    )
  MARK_AS_ADVANCED(NSIS_MAKENSIS)
  FIND_PROGRAM(WINZIP_WZZIP NAMES wzzip
    PATHS   "C:/Program Files/WinZip"
    DOC "Where is makensis.exe located"
    )
  MARK_AS_ADVANCED(WINZIP_WZZIP)
  STRING(REGEX REPLACE "/" "\\\\" CMake_INSTALL_TOP "${CMAKE_INSTALL_PREFIX}")
  SET(NSIS_EXTRA_COMMANDS ";Include system runtime libraries.\n  SetOutPath \"$INSTDIR\\bin\"\n")
  FOREACH(lib ${CMake_INSTALL_SYSTEM_RUNTIME_LIBS})
    STRING(REGEX REPLACE "/" "\\\\" LIB "${lib}")
    SET(NSIS_EXTRA_COMMANDS "${NSIS_EXTRA_COMMANDS}  File \"${LIB}\"\n")
  ENDFOREACH(lib)
  CONFIGURE_FILE(
    ${PROJECT_SOURCE_DIR}/Utilities/Release/${PROJECT_NAME}.nsi.in
    ${PROJECT_BINARY_DIR}/Utilities/Release/${PROJECT_NAME}.nsi 
    IMMEDIATE @ONLY)
  CONFIGURE_FILE(
    ${PROJECT_SOURCE_DIR}/Utilities/Release/Win32Release.sh.in
    ${PROJECT_BINARY_DIR}/Utilities/Release/Win32Release.sh 
    IMMEDIATE @ONLY)
  CONFIGURE_FILE(
    ${PROJECT_SOURCE_DIR}/Utilities/Release/cmake_release.sh.in
    ${PROJECT_BINARY_DIR}/Utilities/Release/cmake_release.sh 
    IMMEDIATE @ONLY)
ENDIF(WIN32)
