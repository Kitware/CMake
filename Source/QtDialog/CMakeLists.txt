PROJECT(QtDialog)
SET(QT_MIN_VERSION "4.3.0")
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
  SET(SRCS
    AddCacheEntry.cxx
    AddCacheEntry.h
    CMakeFirstConfigure.cxx
    CMakeFirstConfigure.h
    CMakeSetup.cxx
    CMakeSetupDialog.cxx
    CMakeSetupDialog.h
    QCMake.cxx
    QCMake.h
    QCMakeCacheView.cxx
    QCMakeCacheView.h
    QCMakeWidgets.cxx
    QCMakeWidgets.h
    QMacInstallDialog.cxx
    QMacInstallDialog.h
    )
  QT4_WRAP_UI(UI_SRCS 
    CMakeFirstConfigure.ui
    CMakeSetupDialog.ui
    AddCacheEntry.ui
    MacInstallDialog.ui
    )
  QT4_WRAP_CPP(MOC_SRCS 
    AddCacheEntry.h
    CMakeFirstConfigure.h
    CMakeSetupDialog.h
    QCMake.h
    QCMakeCacheView.h
    QCMakeWidgets.h
    QMacInstallDialog.h
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
  IF(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 2.4)
    IF(APPLE)
      SET_TARGET_PROPERTIES(cmake-gui PROPERTIES
       OUTPUT_NAME ${CMAKE_BUNDLE_NAME})
    ENDIF(APPLE)
    SET(CMAKE_INSTALL_DESTINATION_ARGS 
      BUNDLE DESTINATION "${CMAKE_BUNDLE_LOCATION}")
  ENDIF(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 2.4)
  # if qt is not static and we are on windows then skip the install
  # I don't want to distribute qt dlls
  IF(CMAKE_PACKAGE_QTGUI)
    INSTALL(TARGETS cmake-gui RUNTIME DESTINATION bin
    ${CMAKE_INSTALL_DESTINATION_ARGS})
  ENDIF(CMAKE_PACKAGE_QTGUI)

  IF(UNIX)
    # install a desktop file so CMake appears in the application start menu
    # with an icon
    INSTALL(FILES CMake.desktop DESTINATION share/applications )
    INSTALL(FILES CMakeSetup.png DESTINATION share/pixmaps )
    INSTALL(FILES cmakecache.xml DESTINATION share/mime/packages )
  ENDIF(UNIX)

  IF(APPLE)
    SET(CMAKE_POSTFLIGHT_SCRIPT
      "${CMake_BINARY_DIR}/Source/QtDialog/postflight.sh")
    SET(CMAKE_POSTUPGRADE_SCRIPT
      "${CMake_BINARY_DIR}/Source/QtDialog/postupgrade.sh")
    configure_file("${CMake_SOURCE_DIR}/Source/QtDialog/postflight.sh.in"
      "${CMake_BINARY_DIR}/Source/QtDialog/postflight.sh")
    configure_file("${CMake_SOURCE_DIR}/Source/QtDialog/postupgrade.sh.in"
      "${CMake_BINARY_DIR}/Source/QtDialog/postupgrade.sh")
    INSTALL(CODE "execute_process(COMMAND ln -s \"../MacOS/${CMAKE_BUNDLE_NAME}\" cmake-gui
                  WORKING_DIRECTORY \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/bin)")
    INSTALL(CODE "set(input_file
       \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/MacOS/${CMAKE_BUNDLE_NAME}\")")
    INSTALL(SCRIPT "${CMake_SOURCE_DIR}/Source/QtDialog/CMakeIngestOSXBundleLibraries.cmake")
  ENDIF(APPLE)
  CONFIGURE_FILE("${QtDialog_SOURCE_DIR}/QtDialogCPack.cmake.in"
    "${QtDialog_BINARY_DIR}/QtDialogCPack.cmake" @ONLY)    
ENDIF(NOT QT4_FOUND)

