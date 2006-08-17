# Hack for Visual Studio support
# Search for system runtime libraries based on the platform.  This is
# not complete because it is used only for the release process by the
# developers.
IF(MSVC)
  FILE(TO_CMAKE_PATH "$ENV{SYSTEMROOT}" SYSTEMROOT)
  IF(MSVC70)
    SET(__install__libs
      "${SYSTEMROOT}/system32/msvcp70.dll"
      "${SYSTEMROOT}/system32/msvcr70.dll"
      )
  ENDIF(MSVC70)
  IF(MSVC71)
    SET(__install__libs
      "${SYSTEMROOT}/system32/msvcp71.dll"
      "${SYSTEMROOT}/system32/msvcr71.dll"
      )
  ENDIF(MSVC71)
  IF(MSVC80)
    # Find the runtime library redistribution directory.
    FIND_PATH(MSVC80_REDIST_DIR NAMES x86/Microsoft.VC80.CRT/Microsoft.VC80.CRT.manifest
      PATHS "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\8.0;InstallDir]/../../VC/redist"
      )
    MARK_AS_ADVANCED(MSVC80_REDIST_DIR)
    SET(MSVC80_CRT_DIR "${MSVC80_REDIST_DIR}/x86/Microsoft.VC80.CRT")

    # Install the manifest that allows DLLs to be loaded from the
    # directory containing the executable.
    SET(__install__libs
      "${MSVC80_CRT_DIR}/Microsoft.VC80.CRT.manifest"
      "${MSVC80_CRT_DIR}/msvcm80.dll"
      "${MSVC80_CRT_DIR}/msvcp80.dll"
      "${MSVC80_CRT_DIR}/msvcr80.dll"
      )
  ENDIF(MSVC80)
  IF(CMAKE_INSTALL_MFC_LIBRARIES)
    IF(MSVC70)
      SET(__install__libs ${__install__libs}
        "${SYSTEMROOT}/system32/mfc70.dll"
        )
    ENDIF(MSVC70)
    IF(MSVC71)
      SET(__install__libs ${__install__libs}
        "${SYSTEMROOT}/system32/mfc71.dll"
        )
    ENDIF(MSVC71)
    IF(MSVC80)
      SET(MSVC80_MFC_DIR "${MSVC80_REDIST_DIR}/x86/Microsoft.VC80.MFC")
      # Install the manifest that allows DLLs to be loaded from the
      # directory containing the executable.
      SET(__install__libs ${__install__libs}
        "${MSVC80_MFC_DIR}/Microsoft.VC80.MFC.manifest"
        "${MSVC80_MFC_DIR}/mfc80.dll"
        )
    ENDIF(MSVC80)
  ENDIF(CMAKE_INSTALL_MFC_LIBRARIES)
  FOREACH(lib
      ${__install__libs}
      )
    IF(EXISTS ${lib})
      SET(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS
        ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} ${lib})
    ENDIF(EXISTS ${lib})
  ENDFOREACH(lib)
ENDIF(MSVC)

# Include system runtime libraries in the installation if any are
# specified by CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS.
IF(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
  IF(WIN32)
    INSTALL_PROGRAMS(/bin ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})
  ELSE(WIN32)
    INSTALL_PROGRAMS(/lib ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})
  ENDIF(WIN32)
ENDIF(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)


