PROJECT(QtDialog)
SET(QT_MIN_VERSION "4.2.0")
FIND_PACKAGE(Qt4 REQUIRED)

IF(NOT QT4_FOUND)
  MESSAGE(SEND_ERROR "Failed to find Qt 4.3 or greater.")
ELSE(NOT QT4_FOUND)

  INCLUDE(${QT_USE_FILE})
  SET(CMAKE_PACKAGE_QTGUI TRUE)
  # i don't want to install or package the qt gui on windows 
  # unless qt is static
  IF(WIN32 AND NOT QT_CONFIG MATCHES "static")
    SET(CMAKE_PACKAGE_QTGUI FALSE)
  ENDIF(WIN32 AND NOT QT_CONFIG MATCHES "static")
  CONFIGURE_FILE("${QtDialog_SOURCE_DIR}/QtDialogCPack.cmake.in"
    "${QtDialog_BINARY_DIR}/QtDialogCPack.cmake")    
  SET(SRCS
    AddCacheEntry.cxx
    AddCacheEntry.h
    CMakeSetup.cxx
    CMakeSetupDialog.cxx
    CMakeSetupDialog.h
    QCMake.cxx
    QCMake.h
    QCMakeCacheView.cxx
    QCMakeCacheView.h
    )
  QT4_WRAP_UI(UI_SRCS 
    CMakeSetupDialog.ui
    AddCacheEntry.ui
    )
  QT4_WRAP_CPP(MOC_SRCS 
    AddCacheEntry.h
    CMakeSetupDialog.h
    QCMake.h
    QCMakeCacheView.h
    )
  QT4_ADD_RESOURCES(RC_SRCS CMakeSetup.qrc)

  SET(SRCS ${SRCS} ${UI_SRCS} ${MOC_SRCS} ${RC_SRCS})
  IF(Q_WS_WIN)
    SET(SRCS ${SRCS} CMakeSetup.rc)
  ENDIF(Q_WS_WIN)
  IF(Q_WS_MAC)
    SET(SRCS ${SRCS} CMakeSetup.icns)
    SET(MACOSX_BUNDLE_ICON_FILE CMakeSetup.icns)
    SET_SOURCE_FILES_PROPERTIES(CMakeSetup.icns PROPERTIES
      MACOSX_PACKAGE_LOCATION Resources)
  ENDIF(Q_WS_MAC)

  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

  ADD_EXECUTABLE(cmake-gui WIN32 MACOSX_BUNDLE ${SRCS})
  TARGET_LINK_LIBRARIES(cmake-gui CMakeLib ${QT_QTMAIN_LIBRARY} ${QT_LIBRARIES})
  # if qt is not static and we are on windows then skip the install
  # I don't want to distribute qt dlls
  IF(CMAKE_PACKAGE_QTGUI)
    INSTALL_TARGETS(/bin cmake-gui)
  ENDIF(CMAKE_PACKAGE_QTGUI)

ENDIF(NOT QT4_FOUND)

