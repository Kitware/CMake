# Locate Qt include paths and libraries

# This module defines
# QT_INCLUDE_PATH, where to find qt.h, etc.
# QT_QT_LIBRARY, where to find the qt library
# QT_MOC_EXE, where to find the moc tool
# QT_UIC_EXE, where to find the uic tool
# USE_QT_FILE, a file for any CMakeLists.txt file to include to actually link against qt
# QT_WRAP_CPP, This allows the QT_WRAP_CPP command to work.
# QT_WRAP_UI, This allows the QT_WRAP_UI command to work.


IF (UNIX)

  FIND_PATH(QT_INCLUDE_PATH qt.h
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

  FIND_FILE(QT_MOC_EXE moc
    ${QTDIR}/bin
    ${path}
  )

  FIND_FILE(QT_UIC_EXE uic
    ${QTDIR}/bin
    ${path}
  )

ENDIF (UNIX)

IF (WIN32)
  # Not sure where to look for Qt under windows
  # Assume that QTDIR has been set

  FIND_PATH(QT_INCLUDE_PATH qt.h
    ${QTDIR}/include
  )

  FIND_LIBRARY(QT_QT_LIBRARY qt
    ${QTDIR}/lib
  )

  FIND_FILE(QT_MOC_EXE moc.exe
    ${QTDIR}/bin
    ${path}
  )

  FIND_FILE(QT_MOC_EXE uic.exe
    ${QTDIR}/bin
    ${path}
  )

ENDIF (WIN32)


IF (QT_MOC_EXE)
  SET ( QT_WRAP_CPP 1 CACHE BOOL "Can we honour the QT_WRAP_CPP command" )
ENDIF (QT_MOC_EXE)

IF (QT_UIC_EXE)
  SET ( QT_WRAP_UI 1 CACHE BOOL "Can we honour the QT_WRAP_UI command" )
ENDIF (QT_UIC_EXE)




