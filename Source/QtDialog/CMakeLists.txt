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
if(POLICY CMP0020)
  cmake_policy(SET CMP0020 NEW) # Drop when CMake >= 2.8.11 required
endif()
find_package(Qt5Widgets QUIET)
if (Qt5Widgets_FOUND)
  include_directories(${Qt5Widgets_INCLUDE_DIRS})
  add_definitions(${Qt5Widgets_DEFINITONS})
  macro(qt4_wrap_ui)
    qt5_wrap_ui(${ARGN})
  endmacro()
  macro(qt4_wrap_cpp)
    qt5_wrap_cpp(${ARGN})
  endmacro()
  macro(qt4_add_resources)
    qt5_add_resources(${ARGN})
  endmacro()
  set(CMake_QT_LIBRARIES ${Qt5Widgets_LIBRARIES})
  set(QT_QTMAIN_LIBRARY ${Qt5Core_QTMAIN_LIBRARIES})

  # Remove this when the minimum version of Qt is 4.6.
  add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

  if(WIN32 AND TARGET Qt5::Core)
    get_property(_Qt5_Core_LOCATION TARGET Qt5::Core PROPERTY LOCATION)
    get_filename_component(Qt_BIN_DIR "${_Qt5_Core_LOCATION}" PATH)
  endif()
else()
  set(QT_MIN_VERSION "4.4.0")
  find_package(Qt4 REQUIRED)
  if(NOT QT4_FOUND)
    message(SEND_ERROR "Failed to find Qt 4.4 or greater.")
    return()
  endif()

  include(${QT_USE_FILE})

  set(CMake_QT_LIBRARIES ${QT_LIBRARIES})

  if(WIN32 AND EXISTS "${QT_QMAKE_EXECUTABLE}")
    get_filename_component(_Qt_BIN_DIR "${QT_QMAKE_EXECUTABLE}" PATH)
    if(EXISTS "${_Qt_BIN_DIR}/QtCore4.dll")
      set(Qt_BIN_DIR ${_Qt_BIN_DIR})
    endif()
  endif()
endif()

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
if(WIN32)
  set(SRCS ${SRCS} CMakeSetup.rc)
endif()
if(APPLE)
  set(SRCS ${SRCS} CMakeSetup.icns)
  set(MACOSX_BUNDLE_ICON_FILE CMakeSetup.icns)
  set_source_files_properties(CMakeSetup.icns PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources)
endif()

if(CMake_GUI_DISTRIBUTE_WITH_Qt_LGPL)
  install(FILES ${CMake_SOURCE_DIR}/Licenses/LGPLv2.1.txt
    DESTINATION ${CMAKE_DATA_DIR}/Licenses)
  set_property(SOURCE CMakeSetupDialog.cxx
    PROPERTY COMPILE_DEFINITIONS CMake_GUI_DISTRIBUTE_WITH_Qt_LGPL)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(cmake-gui WIN32 MACOSX_BUNDLE ${SRCS})
target_link_libraries(cmake-gui CMakeLib ${QT_QTMAIN_LIBRARY} ${CMake_QT_LIBRARIES})
if(Qt_BIN_DIR)
  set_property(TARGET cmake-gui PROPERTY Qt_BIN_DIR ${Qt_BIN_DIR})
endif()

if(APPLE)
  file(STRINGS "${CMake_SOURCE_DIR}/Copyright.txt" copyright_line
    LIMIT_COUNT 1 REGEX "^Copyright 2000-20[0-9][0-9] Kitware")

  set_target_properties(cmake-gui PROPERTIES
    OUTPUT_NAME CMake
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${CMAKE_BUNDLE_VERSION}"
    # TBD: MACOSX_BUNDLE_BUNDLE_VERSION "${CMAKE_BUNDLE_VERSION}"
    MACOSX_BUNDLE_COPYRIGHT "${copyright_line}"
    )

  # Create a symlink in the build tree to provide a "cmake-gui" next
  # to the "cmake" executable that refers to the application bundle.
  add_custom_command(TARGET cmake-gui POST_BUILD
    COMMAND ln -sf CMake.app/Contents/MacOS/CMake
                   $<TARGET_FILE_DIR:cmake>/cmake-gui
    )
endif()
set(CMAKE_INSTALL_DESTINATION_ARGS
  BUNDLE DESTINATION "${CMAKE_BUNDLE_LOCATION}")

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
  install(CODE "execute_process(COMMAND ln -s \"../MacOS/CMake\" cmake-gui
                WORKING_DIRECTORY \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/bin)")
endif()

if(APPLE OR WIN32)
  # install rules for including 3rd party libs such as Qt
  # if a system Qt is used (e.g. installed in /usr/lib/), it will not be included in the installation
  set(fixup_exe "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/bin/cmake-gui${CMAKE_EXECUTABLE_SUFFIX}")
  if(APPLE)
    set(fixup_exe "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/MacOS/CMake")
  endif()
  install(CODE "
    include(\"${CMake_SOURCE_DIR}/Modules/BundleUtilities.cmake\")
    set(BU_CHMOD_BUNDLE_ITEMS ON)
    fixup_bundle(\"${fixup_exe}\" \"\" \"${QT_LIBRARY_DIR};${QT_BINARY_DIR}\")
  ")
endif()

set(CMAKE_PACKAGE_QTGUI TRUE)
configure_file("${QtDialog_SOURCE_DIR}/QtDialogCPack.cmake.in"
  "${QtDialog_BINARY_DIR}/QtDialogCPack.cmake" @ONLY)
