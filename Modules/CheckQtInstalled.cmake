# searches for the installed versions of QT
# This should only be used if your project can work with both qt3 and gt4.
# If not, you should just directly use FindQt4 or FindQt3.
#
# Sets QT3_INSTALLED to TRUE if qt3 is installed
# Sets QT4_INSTALLED to TRUE if qt4 is installed
# So, if you had a project that worked with Qt3 and Qt4, but preferred Qt3, you would do this:

# INCLUDE(CMakeQtInstalled)
# IF(Qt3_INSTALLED)
#   INCLUDE(FindQt3)
# ELSEIF(Qt3_INSTALLED)
#   IF(Qt4_INSTALLED)
#      INCLUDE(FindQt4)
#   ENDIF(Qt4_INSTALLED)
# ENDIF(Qt3_INSTALLED)



# look for signs of qt3 installations
FILE(GLOB GLOB_TEMP_VAR /usr/lib/qt-3*/bin/qmake)
IF(GLOB_TEMP_VAR)
  SET(QT3_INSTALLED TRUE)
ENDIF(GLOB_TEMP_VAR)
SET(GLOB_TEMP_VAR)

FILE(GLOB GLOB_TEMP_VAR /usr/local/qt-x11-commercial-3*/bin/qmake)
IF(GLOB_TEMP_VAR)
  SET(QT3_INSTALLED TRUE)
ENDIF(GLOB_TEMP_VAR)
SET(GLOB_TEMP_VAR)

# look for qt4 installations
FILE(GLOB GLOB_TEMP_VAR /usr/local/qt-x11-commercial-4*/bin/qmake)
IF(GLOB_TEMP_VAR)
  SET(QT4_INSTALLED TRUE)
ENDIF(GLOB_TEMP_VAR)
SET(GLOB_TEMP_VAR)

FILE(GLOB GLOB_TEMP_VAR /usr/local/Trolltech/Qt-4*/bin/qmake)
IF(GLOB_TEMP_VAR)
  SET(QT4_INSTALLED TRUE)
ENDIF(GLOB_TEMP_VAR)
SET(GLOB_TEMP_VAR)

# now find qmake
FIND_PROGRAM(QT_QMAKE_EXECUTABLE NAMES qmake PATHS $ENV{QTDIR}/bin)
IF(QT_QMAKE_EXECUTABLE)
  EXEC_PROGRAM(${QMAKE_PATH} ARGS "-query QT_VERSION"
    OUTPUT_VARIABLE QTVERSION)
  IF(QTVERSION MATCHES "4.*")
    SET(QT4_INSTALLED TRUE)
  ENDIF(QTVERSION MATCHES "4.*")
  IF(QTVERSION MATCHES "Unknown")
    SET(QT3_INSTALLED TRUE)
  ENDIF(QTVERSION MATCHES "Unknown")
ENDIF(QT_QMAKE_EXECUTABLE)

IF(QT_QMAKE_EXECUTABLE)
  EXEC_PROGRAM( ${QT_QMAKE_EXECUTABLE}
    ARGS "-query QT_INSTALL_HEADERS" 
    OUTPUT_VARIABLE qt_headers )
ENDIF(QT_QMAKE_EXECUTABLE)

FIND_FILE( QT4_QGLOBAL_H_FILE qglobal.h
  ${qt_headers}/Qt
  $ENV{QTDIR}/include/Qt
  /usr/local/qt/include/Qt
  /usr/local/include/Qt
  /usr/lib/qt/include/Qt
  /usr/include/Qt
  /usr/share/qt4/include/Qt
  C:/Progra~1/qt/include/Qt )F

IF(QT4_QGLOBAL_H_FILE)
  SET(QT4_INSTALLED TRUE)
ENDIF(QT4_QGLOBAL_H_FILE)

FIND_FILE( QT3_QGLOBAL_H_FILE qglobal.h
  C:/Qt/3.3.3Educational/include
  $ENV{QTDIR}/include
  /usr/include/qt3/Qt
  /usr/local/qt/include
  /usr/local/include
  /usr/lib/qt/include
  /usr/include
  /usr/share/qt3/include
  C:/Progra~1/qt/include
  /usr/include/qt3 )

IF(QT3_QGLOBAL_H_FILE)
  SET(QT3_INSTALLED TRUE)
ENDIF(QT3_QGLOBAL_H_FILE)

