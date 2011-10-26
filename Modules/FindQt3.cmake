# - Locate Qt include paths and libraries
# This module defines:
#  QT_INCLUDE_DIR - where to find qt.h, etc.
#  QT_LIBRARIES   - the libraries to link against to use Qt.
#  QT_DEFINITIONS - definitions to use when
#                   compiling code that uses Qt.
#  QT_FOUND       - If false, don't try to use Qt.
#
# If you need the multithreaded version of Qt, set QT_MT_REQUIRED to TRUE
#
# Also defined, but not for general use are:
#  QT_MOC_EXECUTABLE, where to find the moc tool.
#  QT_UIC_EXECUTABLE, where to find the uic tool.
#  QT_QT_LIBRARY, where to find the Qt library.
#  QT_QTMAIN_LIBRARY, where to find the qtmain
#   library. This is only required by Qt3 on Windows.

# These are around for backwards compatibility
# they will be set
#  QT_WRAP_CPP, set true if QT_MOC_EXECUTABLE is found
#  QT_WRAP_UI set true if QT_UIC_EXECUTABLE is found

#=============================================================================
# Copyright 2005-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

# If Qt4 has already been found, fail.
if(QT4_FOUND)
  if(Qt3_FIND_REQUIRED)
    message( FATAL_ERROR "Qt3 and Qt4 cannot be used together in one project.")
  else(Qt3_FIND_REQUIRED)
    if(NOT Qt3_FIND_QUIETLY)
      message( STATUS    "Qt3 and Qt4 cannot be used together in one project.")
    endif(NOT Qt3_FIND_QUIETLY)
    return()
  endif(Qt3_FIND_REQUIRED)
endif(QT4_FOUND)


file(GLOB GLOB_PATHS_BIN /usr/lib/qt-3*/bin/)
find_path(QT_INCLUDE_DIR qt.h
  "[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\3.2.1;InstallDir]/include/Qt"
  "[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\3.2.0;InstallDir]/include/Qt"
  "[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\3.1.0;InstallDir]/include/Qt"
  $ENV{QTDIR}/include
  ${GLOB_PATHS_BIN}
  /usr/local/qt/include
  /usr/lib/qt/include
  /usr/lib/qt3/include
  /usr/include/qt
  /usr/share/qt3/include
  C:/Progra~1/qt/include
  /usr/include/qt3
  )

# if qglobal.h is not in the qt_include_dir then set
# QT_INCLUDE_DIR to NOTFOUND
if(NOT EXISTS ${QT_INCLUDE_DIR}/qglobal.h)
  set(QT_INCLUDE_DIR QT_INCLUDE_DIR-NOTFOUND CACHE PATH "path to Qt3 include directory" FORCE)
endif(NOT EXISTS ${QT_INCLUDE_DIR}/qglobal.h)

if(QT_INCLUDE_DIR)
  #extract the version string from qglobal.h
  file(READ ${QT_INCLUDE_DIR}/qglobal.h QGLOBAL_H)
  string(REGEX MATCH "#define[\t ]+QT_VERSION_STR[\t ]+\"([0-9]+.[0-9]+.[0-9]+)\"" QGLOBAL_H "${QGLOBAL_H}")
  string(REGEX REPLACE ".*\"([0-9]+.[0-9]+.[0-9]+)\".*" "\\1" qt_version_str "${QGLOBAL_H}")

  # Under windows the qt library (MSVC) has the format qt-mtXYZ where XYZ is the
  # version X.Y.Z, so we need to remove the dots from version
  string(REGEX REPLACE "\\." "" qt_version_str_lib "${qt_version_str}")
endif(QT_INCLUDE_DIR)

file(GLOB GLOB_PATHS_LIB /usr/lib/qt-3*/lib/)
if(QT_MT_REQUIRED)
  find_library(QT_QT_LIBRARY
    NAMES
    qt-mt qt-mt${qt_version_str_lib} qt-mtnc${qt_version_str_lib}
    qt-mtedu${qt_version_str_lib} qt-mt230nc qt-mtnc321 qt-mt3
    PATHS
  "[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\3.2.1;InstallDir]/lib"
  "[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\3.2.0;InstallDir]/lib"
  "[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\3.1.0;InstallDir]/lib"
    $ENV{QTDIR}/lib
    ${GLOB_PATHS_LIB}
    /usr/local/qt/lib
    /usr/lib/qt/lib
    /usr/lib/qt3/lib
    /usr/lib/qt3/lib64
    /usr/share/qt3/lib
    C:/Progra~1/qt/lib
    )

else(QT_MT_REQUIRED)
  find_library(QT_QT_LIBRARY
    NAMES
    qt qt-${qt_version_str_lib} qt-edu${qt_version_str_lib}
    qt-mt qt-mt${qt_version_str_lib} qt-mtnc${qt_version_str_lib}
    qt-mtedu${qt_version_str_lib} qt-mt230nc qt-mtnc321 qt-mt3
    PATHS
    "[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\3.2.1;InstallDir]/lib"
    "[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\3.2.0;InstallDir]/lib"
    "[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\3.1.0;InstallDir]/lib"
    $ENV{QTDIR}/lib
    ${GLOB_PATHS_LIB}
    /usr/local/qt/lib
    /usr/lib/qt/lib
    /usr/lib/qt3/lib
    /usr/lib/qt3/lib64
    /usr/share/qt3/lib
    C:/Progra~1/qt/lib
    )
endif(QT_MT_REQUIRED)


find_library(QT_QASSISTANTCLIENT_LIBRARY
  NAMES qassistantclient
  PATHS
  "[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\3.2.1;InstallDir]/lib"
  "[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\3.2.0;InstallDir]/lib"
  "[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\3.1.0;InstallDir]/lib"
  $ENV{QTDIR}/lib
  ${GLOB_PATHS_LIB}
  /usr/local/qt/lib
  /usr/lib/qt3/lib
  /usr/lib/qt3/lib64
  /usr/share/qt3/lib
  C:/Progra~1/qt/lib
  )

# qt 3 should prefer QTDIR over the PATH
find_program(QT_MOC_EXECUTABLE
  NAMES moc-qt3 moc
  HINTS
  $ENV{QTDIR}/bin
  PATHS
  "[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\3.2.1;InstallDir]/include/Qt"
  "[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\3.2.0;InstallDir]/include/Qt"
  "[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\3.1.0;InstallDir]/include/Qt"
  $ENV{QTDIR}/bin
  ${GLOB_PATHS_BIN}
  /usr/local/qt/bin
  /usr/lib/qt/bin
  /usr/lib/qt3/bin
  /usr/share/qt3/bin
  C:/Progra~1/qt/bin
  /usr/X11R6/bin
  )

if(QT_MOC_EXECUTABLE)
  set( QT_WRAP_CPP "YES")
endif(QT_MOC_EXECUTABLE)

# qt 3 should prefer QTDIR over the PATH
find_program(QT_UIC_EXECUTABLE
  NAMES uic-qt3 uic
  HINTS
  $ENV{QTDIR}/bin
  PATHS
  "[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\3.2.1;InstallDir]/include/Qt"
  "[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\3.2.0;InstallDir]/include/Qt"
  "[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\3.1.0;InstallDir]/include/Qt"
  ${GLOB_PATHS_BIN}
  /usr/local/qt/bin
  /usr/lib/qt/bin
  /usr/lib/qt3/bin
  /usr/share/qt3/bin
  C:/Progra~1/qt/bin
  /usr/X11R6/bin
  )

if(QT_UIC_EXECUTABLE)
  set( QT_WRAP_UI "YES")
endif(QT_UIC_EXECUTABLE)

if(WIN32)
  find_library(QT_QTMAIN_LIBRARY qtmain
    "[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\3.2.1;InstallDir]/lib"
    "[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\3.2.0;InstallDir]/lib"
    "[HKEY_CURRENT_USER\\Software\\Trolltech\\Qt3Versions\\3.1.0;InstallDir]/lib"
    "$ENV{ProgramFiles}/qt/lib"
    $ENV{QTDIR}/lib "C:/Program Files/qt/lib"
    DOC "This Library is only needed by and included with Qt3 on MSWindows. It should be NOTFOUND, undefined or IGNORE otherwise."
    )
endif(WIN32)


if(QT_MIN_VERSION)

  string(REGEX REPLACE "([0-9]+)\\.[0-9]+\\.[0-9]+" "\\1" qt_major_vers "${qt_version_str}")
  string(REGEX REPLACE "[0-9]+\\.([0-9]+)\\.[0-9]+" "\\1" qt_minor_vers "${qt_version_str}")
  string(REGEX REPLACE "[0-9]+\\.[0-9]+\\.([0-9]+)" "\\1" qt_patch_vers "${qt_version_str}")

  #now parse the parts of the user given version string into variables
  string(REGEX MATCH "^[0-9]+\\.[0-9]+\\.[0-9]+$" req_qt_major_vers "${QT_MIN_VERSION}")
  if(NOT req_qt_major_vers)
    error_message(  "Invalid Qt version string given: \"${QT_MIN_VERSION}\", expected e.g. \"3.1.5\"")
  endif(NOT req_qt_major_vers)

  string(REGEX REPLACE "([0-9]+)\\.[0-9]+\\.[0-9]+" "\\1" req_qt_major_vers "${QT_MIN_VERSION}")
  string(REGEX REPLACE "[0-9]+\\.([0-9])+\\.[0-9]+" "\\1" req_qt_minor_vers "${QT_MIN_VERSION}")
  string(REGEX REPLACE "[0-9]+\\.[0-9]+\\.([0-9]+)" "\\1" req_qt_patch_vers "${QT_MIN_VERSION}")

  # req = "6.5.4", qt = "3.2.1"
  macro(error_message msg)
    if(QT3_REQUIRED)
      message( FATAL_ERROR ${msg})
    else(QT3_REQUIRED)
      message( STATUS ${msg})
    endif(QT3_REQUIRED)
  endmacro(error_message)

  if(req_qt_major_vers GREATER qt_major_vers)                  # (6 > 3) ?
    ERROR_MESSAGE(  "Qt major version not matched (required: ${QT_MIN_VERSION}, found: ${qt_version_str})")            # yes
  else(req_qt_major_vers GREATER qt_major_vers)               # no
    if(req_qt_major_vers LESS qt_major_vers)                  # (6 < 3) ?
      set( QT_VERSION_BIG_ENOUGH "YES" )                      # yes
    else(req_qt_major_vers LESS qt_major_vers)                # ( 6==3) ?
      if(req_qt_minor_vers GREATER qt_minor_vers)            # (5>2) ?
        ERROR_MESSAGE(  "Qt minor version not matched (required: ${QT_MIN_VERSION}, found: ${qt_version_str})")      # yes
      else(req_qt_minor_vers GREATER qt_minor_vers)          # no
        if(req_qt_minor_vers LESS qt_minor_vers)            # (5<2) ?
          set( QT_VERSION_BIG_ENOUGH "YES" )                # yes
        else(req_qt_minor_vers LESS qt_minor_vers)          # (5==2)
          if(req_qt_patch_vers GREATER qt_patch_vers)      # (4>1) ?
            ERROR_MESSAGE(  "Qt patch level not matched (required: ${QT_MIN_VERSION}, found: ${qt_version_str})")  # yes
          else(req_qt_patch_vers GREATER qt_patch_vers)    # (4>1) ?
            set( QT_VERSION_BIG_ENOUGH "YES" )             # yes
          endif(req_qt_patch_vers GREATER qt_patch_vers)   # (4>1) ?
        endif(req_qt_minor_vers LESS qt_minor_vers)
      endif(req_qt_minor_vers GREATER qt_minor_vers)
    endif(req_qt_major_vers LESS qt_major_vers)
  endif(req_qt_major_vers GREATER qt_major_vers)
endif(QT_MIN_VERSION)

# if the include a library are found then we have it
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Qt3  DEFAULT_MSG  QT_QT_LIBRARY QT_INCLUDE_DIR QT_MOC_EXECUTABLE)
set(QT_FOUND ${QT3_FOUND} )

if(QT_FOUND)
  set( QT_LIBRARIES ${QT_LIBRARIES} ${QT_QT_LIBRARY} )
  set( QT_DEFINITIONS "")

  if(WIN32 AND NOT CYGWIN)
    if(QT_QTMAIN_LIBRARY)
      # for version 3
      set(QT_DEFINITIONS -DQT_DLL -DQT_THREAD_SUPPORT -DNO_DEBUG)
      set(QT_LIBRARIES imm32.lib ${QT_QT_LIBRARY} ${QT_QTMAIN_LIBRARY} )
      set(QT_LIBRARIES ${QT_LIBRARIES} winmm wsock32)
    else(QT_QTMAIN_LIBRARY)
      # for version 2
      set(QT_LIBRARIES imm32.lib ws2_32.lib ${QT_QT_LIBRARY} )
    endif(QT_QTMAIN_LIBRARY)
  else(WIN32 AND NOT CYGWIN)
    set(QT_LIBRARIES ${QT_QT_LIBRARY} )

    set(QT_DEFINITIONS -DQT_SHARED -DQT_NO_DEBUG)
    if(QT_QT_LIBRARY MATCHES "qt-mt")
      set(QT_DEFINITIONS ${QT_DEFINITIONS} -DQT_THREAD_SUPPORT -D_REENTRANT)
    endif(QT_QT_LIBRARY MATCHES "qt-mt")

  endif(WIN32 AND NOT CYGWIN)

  if(QT_QASSISTANTCLIENT_LIBRARY)
    set(QT_LIBRARIES ${QT_QASSISTANTCLIENT_LIBRARY} ${QT_LIBRARIES})
  endif(QT_QASSISTANTCLIENT_LIBRARY)

  # Backwards compatibility for CMake1.4 and 1.2
  set(QT_MOC_EXE ${QT_MOC_EXECUTABLE} )
  set(QT_UIC_EXE ${QT_UIC_EXECUTABLE} )
  # for unix add X11 stuff
  if(UNIX)
    find_package(X11)
    if(X11_FOUND)
      set(QT_LIBRARIES ${QT_LIBRARIES} ${X11_LIBRARIES})
    endif(X11_FOUND)
    if(CMAKE_DL_LIBS)
      set(QT_LIBRARIES ${QT_LIBRARIES} ${CMAKE_DL_LIBS})
    endif(CMAKE_DL_LIBS)
  endif(UNIX)
  if(QT_QT_LIBRARY MATCHES "qt-mt")
    find_package(Threads)
    set(QT_LIBRARIES ${QT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
  endif(QT_QT_LIBRARY MATCHES "qt-mt")
endif(QT_FOUND)

exec_program(${QT_MOC_EXECUTABLE} ARGS "-v" OUTPUT_VARIABLE QTVERSION_MOC)
exec_program(${QT_UIC_EXECUTABLE} ARGS "-version" OUTPUT_VARIABLE QTVERSION_UIC)

set(_QT_UIC_VERSION_3 FALSE)
if("${QTVERSION_UIC}" MATCHES ".* 3..*")
  set(_QT_UIC_VERSION_3 TRUE)
endif("${QTVERSION_UIC}" MATCHES ".* 3..*")

set(_QT_MOC_VERSION_3 FALSE)
if("${QTVERSION_MOC}" MATCHES ".* 3..*")
  set(_QT_MOC_VERSION_3 TRUE)
endif("${QTVERSION_MOC}" MATCHES ".* 3..*")

set(QT_WRAP_CPP FALSE)
if(QT_MOC_EXECUTABLE AND _QT_MOC_VERSION_3)
  set( QT_WRAP_CPP TRUE)
endif(QT_MOC_EXECUTABLE AND _QT_MOC_VERSION_3)

set(QT_WRAP_UI FALSE)
if(QT_UIC_EXECUTABLE AND _QT_UIC_VERSION_3)
  set( QT_WRAP_UI TRUE)
endif(QT_UIC_EXECUTABLE AND _QT_UIC_VERSION_3)

mark_as_advanced(
  QT_INCLUDE_DIR
  QT_QT_LIBRARY
  QT_QTMAIN_LIBRARY
  QT_QASSISTANTCLIENT_LIBRARY
  QT_UIC_EXECUTABLE
  QT_MOC_EXECUTABLE
  QT_WRAP_CPP
  QT_WRAP_UI
  )
