# Locate Qt include paths and libraries

# This module defines
# QT_INCLUDE_DIR, where to find qt.h, etc.
# QT_LIBRARIES, the libraries to link against to use Qt.
# QT_DEFINITIONS, definitions to use when compiling code that uses Qt.
# QT_WRAP_CPP, If false, don't use QT_WRAP_CPP command.
# QT_WRAP_UI, If false, don't use QT_WRAP_UI command.
# QT_FOUND, If false, do try to use Qt.

# also defined, but not for general use are
# QT_MOC_EXECUTABLE, where to find the moc tool.
# QT_UIC_EXECUTABLE, where to find the uic tool.
# QT_QT_LIBRARY, where to find the Qt library.
# QT_QTMAIN_LIBRARY, where to find the qtmain library. This is only required by Qt3 on Windows.

IF (UNIX)

  FIND_PATH(QT_INCLUDE_DIR qt.h
    ${QTDIR}/include
    /usr/local/qt/include
    /usr/local/include
    /usr/include
  )

  FIND_LIBRARY(QT_QT_LIBRARY qt
    ${QTDIR}/lib
    /usr/local/qt/lib
    /usr/local/lib
    /usr/lib
  )

  FIND_FILE(QT_MOC_EXECUTABLE moc
    ${QTDIR}/bin
    ${path}
  )

  FIND_FILE(QT_UIC_EXECUTABLE uic
    ${QTDIR}/bin
    ${path}
  )

ENDIF (UNIX)

IF (WIN32)
  # Not sure where to look for Qt under windows
  # Assume that QTDIR has been set

  FIND_PATH(QT_INCLUDE_DIR qt.h
    ${QTDIR}/include C:/Progra~1/qt/include) )

  FIND_LIBRARY(QT_QT_LIBRARY qt
    ${QTDIR}/lib C:/Progra~1/qt/lib )

  FIND_LIBRARY(QT_QTMAIN_LIBRARY qtmain
    ${QTDIR}/lib C:/Progra~1/qt/lib
    DOC "This Library is only needed by and included with Qt3 on MSWindows. It should be NOTFOUND, undefined or IGNORE otherwise."
  )

  FIND_FILE(QT_MOC_EXECUTABLE moc.exe
    ${QTDIR}/bin C:/Progra~1/qt/bin
    ${path} )

  FIND_FILE(QT_UIC_EXECUTABLE uic.exe
    ${QTDIR}/bin C:/Progra~1/qt/bin
    ${path} )

ENDIF (WIN32)


IF (QT_MOC_EXECUTABLE)
  SET ( QT_WRAP_CPP "YES")
ENDIF (QT_MOC_EXECUTABLE)

IF (QT_UIC_EXECUTABLE)
  SET ( QT_WRAP_UI "YES")
ENDIF (QT_UIC_EXECUTABLE)


IF(QT_INCLUDE_DIR)
  IF(QT_QT_LIBRARY)
    SET( QT_LIBRARIES ${QT_LIBRARIES} ${QT_QT_LIBRARY} )
    SET( QT_FOUND "YES" )
    SET( QT_DEFINITIONS "")

    IF (WIN32)
      IF (QT_QTMAIN_LIBRARY)
        # for version 3
        SET (QT_DEFINITIONS -DQT_DLL)
        SET (QT_LIBRARIES imm32.lib  ${QT_QT_LIBRARY} ${QT_QTMAIN_LIBRARY} )
      ELSE (QT_QTMAIN_LIBRARY)
        # for version 2
        SET (QT_LIBRARIES imm32.lib ws2_32.lib  ${QT_QT_LIBRARY} )
      ENDIF (QT_QTMAIN_LIBRARY)
    ELSE (WIN32)
      SET (QT_LIBRARIES ${QT_QT_LIBRARY} )
    ENDIF (WIN32)

# Backwards compatibility for CMake1.4 and 1.2
    SET (QT_MOC_EXE ${QT_MOC_EXECUTABLE} )
    SET (QT_UIC_EXE ${QT_UIC_EXECUTABLE} )

  ENDIF(QT_QT_LIBRARY)
ENDIF(QT_INCLUDE_DIR)


