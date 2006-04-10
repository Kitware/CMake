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
    SET(__install__libs
      "${SYSTEMROOT}/system32/msvcp80.dll"
      "${SYSTEMROOT}/system32/msvcr80.dll"
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
      SET(__install__libs ${__install__libs}
        "${SYSTEMROOT}/system32/mfc80.dll"
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


