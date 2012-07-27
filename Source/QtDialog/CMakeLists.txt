#=============================================================================
# CMake - Cross Platform Makefile Generator
# Copyright 2000-2009 Kitware, Inc., Insight Software Consortium
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
PROJECT(QtDialog)
SET(QT_MIN_VERSION "4.4.0")
FIND_PACKAGE(Qt4 REQUIRED)

IF(NOT QT4_FOUND)
  MESSAGE(SEND_ERROR "Failed to find Qt 4.4 or greater.")
ELSE(NOT QT4_FOUND)

  INCLUDE(${QT_USE_FILE})
  SET(CMAKE_PACKAGE_QTGUI TRUE)
  SET(SRCS
    AddCacheEntry.cxx
    AddCacheEntry.h
    CMakeSetup.cxx
    CMakeSetupDialog.cxx
    CMakeSetupDialog.h
    FirstConfigure.cxx
    FirstConfigure.h
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
    CMakeSetupDialog.ui
    Compilers.ui
    CrossCompiler.ui
    AddCacheEntry.ui
    MacInstallDialog.ui
    )
  QT4_WRAP_CPP(MOC_SRCS
    AddCacheEntry.h
    Compilers.h
    CMakeSetupDialog.h
    FirstConfigure.h
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

  INSTALL(TARGETS cmake-gui RUNTIME DESTINATION bin ${CMAKE_INSTALL_DESTINATION_ARGS})

  IF(UNIX)
    # install a desktop file so CMake appears in the application start menu
    # with an icon
    INSTALL(FILES CMake.desktop DESTINATION share/applications )
    INSTALL(FILES CMakeSetup32.png DESTINATION share/pixmaps )
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
  ENDIF(APPLE)

  if(APPLE OR WIN32)
    # install rules for including 3rd party libs such as Qt
    # if a system Qt is used (e.g. installed in /usr/lib/), it will not be included in the installation
    set(fixup_exe "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/bin/cmake-gui${CMAKE_EXECUTABLE_SUFFIX}")
    if(APPLE)
      set(fixup_exe "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/MacOS/${CMAKE_BUNDLE_NAME}")
    endif(APPLE)
    install(CODE "
      include(\"${CMake_SOURCE_DIR}/Modules/BundleUtilities.cmake\")
      set(BU_CHMOD_BUNDLE_ITEMS ON)
      fixup_bundle(\"${fixup_exe}\" \"\" \"${QT_LIBRARY_DIR};${QT_BINARY_DIR}\")
    ")
  endif(APPLE OR WIN32)

  CONFIGURE_FILE("${QtDialog_SOURCE_DIR}/QtDialogCPack.cmake.in"
    "${QtDialog_BINARY_DIR}/QtDialogCPack.cmake" @ONLY)
ENDIF(NOT QT4_FOUND)

