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
project(QtDialog)
set(QT_MIN_VERSION "4.4.0")
find_package(Qt4 REQUIRED)

if(NOT QT4_FOUND)
  message(SEND_ERROR "Failed to find Qt 4.4 or greater.")
else()

  include(${QT_USE_FILE})
  set(CMAKE_PACKAGE_QTGUI TRUE)
  set(SRCS
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

  set(SRCS ${SRCS} ${UI_SRCS} ${MOC_SRCS} ${RC_SRCS})
  if(Q_WS_WIN)
    set(SRCS ${SRCS} CMakeSetup.rc)
  endif()
  if(Q_WS_MAC)
    set(SRCS ${SRCS} CMakeSetup.icns)
    set(MACOSX_BUNDLE_ICON_FILE CMakeSetup.icns)
    set_source_files_properties(CMakeSetup.icns PROPERTIES
      MACOSX_PACKAGE_LOCATION Resources)
  endif()

  include_directories(${CMAKE_CURRENT_BINARY_DIR})
  include_directories(${CMAKE_CURRENT_SOURCE_DIR})

  add_executable(cmake-gui WIN32 MACOSX_BUNDLE ${SRCS})
  target_link_libraries(cmake-gui CMakeLib ${QT_QTMAIN_LIBRARY} ${QT_LIBRARIES})
  if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 2.4)
    if(APPLE)
      set_target_properties(cmake-gui PROPERTIES
       OUTPUT_NAME ${CMAKE_BUNDLE_NAME})
    endif()
    set(CMAKE_INSTALL_DESTINATION_ARGS
      BUNDLE DESTINATION "${CMAKE_BUNDLE_LOCATION}")
  endif()

  install(TARGETS cmake-gui RUNTIME DESTINATION bin ${CMAKE_INSTALL_DESTINATION_ARGS})

  if(UNIX)
    # install a desktop file so CMake appears in the application start menu
    # with an icon
    install(FILES CMake.desktop DESTINATION share/applications )
    install(FILES CMakeSetup32.png DESTINATION share/pixmaps )
    install(FILES cmakecache.xml DESTINATION share/mime/packages )
  endif()

  if(APPLE)
    set(CMAKE_POSTFLIGHT_SCRIPT
      "${CMake_BINARY_DIR}/Source/QtDialog/postflight.sh")
    set(CMAKE_POSTUPGRADE_SCRIPT
      "${CMake_BINARY_DIR}/Source/QtDialog/postupgrade.sh")
    configure_file("${CMake_SOURCE_DIR}/Source/QtDialog/postflight.sh.in"
      "${CMake_BINARY_DIR}/Source/QtDialog/postflight.sh")
    configure_file("${CMake_SOURCE_DIR}/Source/QtDialog/postupgrade.sh.in"
      "${CMake_BINARY_DIR}/Source/QtDialog/postupgrade.sh")
    install(CODE "execute_process(COMMAND ln -s \"../MacOS/${CMAKE_BUNDLE_NAME}\" cmake-gui
                  WORKING_DIRECTORY \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/bin)")
  endif()

  if(APPLE OR WIN32)
    # install rules for including 3rd party libs such as Qt
    # if a system Qt is used (e.g. installed in /usr/lib/), it will not be included in the installation
    set(fixup_exe "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/bin/cmake-gui${CMAKE_EXECUTABLE_SUFFIX}")
    if(APPLE)
      set(fixup_exe "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/MacOS/${CMAKE_BUNDLE_NAME}")
    endif()
    install(CODE "
      include(\"${CMake_SOURCE_DIR}/Modules/BundleUtilities.cmake\")
      set(BU_CHMOD_BUNDLE_ITEMS ON)
      fixup_bundle(\"${fixup_exe}\" \"\" \"${QT_LIBRARY_DIR};${QT_BINARY_DIR}\")
    ")
  endif()

  configure_file("${QtDialog_SOURCE_DIR}/QtDialogCPack.cmake.in"
    "${QtDialog_BINARY_DIR}/QtDialogCPack.cmake" @ONLY)
endif()

