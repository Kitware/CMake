# Hack for Visual Studio support
# Search for system runtime libraries based on the platform.  This is
# not complete because it is used only for the release process by the
# developers.
IF(MSVC)
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
ENDIF(MSVC)

# Include system runtime libraries in the installation if any are
# specified by CMake_INSTALL_SYSTEM_RUNTIME_LIBS.
IF(CMake_INSTALL_SYSTEM_RUNTIME_LIBS)
  IF(WIN32)
    INSTALL_PROGRAMS(/bin ${CMake_INSTALL_SYSTEM_RUNTIME_LIBS})
  ELSE(WIN32)
    INSTALL_PROGRAMS(/lib ${CMake_INSTALL_SYSTEM_RUNTIME_LIBS})
  ENDIF(WIN32)
ENDIF(CMake_INSTALL_SYSTEM_RUNTIME_LIBS)


