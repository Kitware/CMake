# searches for all installed versions of QT.
# This should only be used if your project can work with multiple
# versions of QT.  If not, you should just directly use FindQt4 or FindQt3.
# 
#  If multiple versions of QT are found on the machine, then
#  The user must set the option DESIRED_QT_VERSION to the version
#  they want to use.  If only one version of qt is found on the machine,
#  then the DESIRED_QT_VERSION is set to that version and the 
#  matching FindQt3 or FindQt4 module is included.
#  Once the user sets DESIRED_QT_VERSION, then the FindQt3 or FindQt4 module
#  is included.
#
#  DESIRED_QT_VERSION OPTION is created
#  QT4_INSTALLED is set to TRUE if qt4 is found.
#  QT3_INSTALLED is set to TRUE if qt3 is found.

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
  "[HKEY_CURRENT_USER\Software\Trolltech\Qt3Versions\4.0.0;InstallDir]/include/Qt"
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
  "[HKEY_CURRENT_USER\Software\Trolltech\Qt3Versions\3.2.1;InstallDir]/include/Qt"
  "[HKEY_CURRENT_USER\Software\Trolltech\Qt3Versions\3.2.0;InstallDir]/include/Qt"
  "[HKEY_CURRENT_USER\Software\Trolltech\Qt3Versions\3.1.0;InstallDir]/include/Qt"
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

IF(QT3_INSTALLED AND QT4_INSTALLED )
  # force user to pick if we have both
  OPTION(DESIRED_QT_VERSION "Pick a version of QT to use" 0)
ELSE(QT3_INSTALLED AND QT4_INSTALLED )
  # if only one found then pick that one
  IF(QT3_INSTALLED)
    OPTION(DESIRED_QT_VERSION "Pick a version of QT to use" 3)
  ENDIF(QT3_INSTALLED)
  IF(QT4_INSTALLED)
    OPTION(DESIRED_QT_VERSION "Pick a version of QT to use" 4)
  ENDIF(QT4_INSTALLED)
ENDIF(QT3_INSTALLED AND QT4_INSTALLED )

IF(DESIRED_QT_VERSION MATCHES 3)
  INCLUDE(FindQt3)
ENDIF(DESIRED_QT_VERSION MATCHES 3)
IF(DESIRED_QT_VERSION MATCHES 4)
  INCLUDE(FindQt4)
ENDIF(DESIRED_QT_VERSION MATCHES 4)

IF(NOT QT3_INSTALLED AND NOT QT4_INSTALLED)
  MESSAGE(SEND_ERROR "CMake was unable to find any QT versions, put qmake in your path, or set QT_QMAKE_EXECUTABLE.")
ELSE(NOT QT3_INSTALLED AND NOT QT4_INSTALLED)
  IF(NOT QT_FOUND AND NOT DESIRED_QT_VERSION)
    MESSAGE(SEND_ERROR "Multiple versions of QT found please set DESIRED_QT_VERSION")
  ENDIF(NOT QT_FOUND AND NOT DESIRED_QT_VERSION)
  IF(NOT QT_FOUND AND DESIRED_QT_VERSION)
    MESSAGE(SEND_ERROR "CMake was unable to find QT version: ${DESIRED_QT_VERSION}")
  ENDIF(NOT QT_FOUND AND DESIRED_QT_VERSION)
ENDIF(NOT QT3_INSTALLED AND NOT QT4_INSTALLED)
