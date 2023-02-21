# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

project(QtDialog)
CMake_OPTIONAL_COMPONENT(cmake-gui)
set(QT_COMPONENTS
  Core
  Widgets
  Gui
)

set(CMake_QT_MAJOR_VERSION "A" CACHE
  STRING "Expected Qt major version. Valid values are A (auto-select), 5, 6.")
set(SUPPORTED_QT_VERSIONS "A" 5 6)
set_property(CACHE CMake_QT_MAJOR_VERSION PROPERTY STRINGS ${SUPPORTED_QT_VERSIONS})
if(NOT CMake_QT_MAJOR_VERSION STREQUAL "A")
  if(NOT CMake_QT_MAJOR_VERSION IN_LIST SUPPORTED_QT_VERSIONS)
    message(FATAL_ERROR "Supported Qt versions are \"${SUPPORTED_QT_VERSIONS}\"."
            " But CMake_QT_MAJOR_VERSION is set to ${CMake_QT_MAJOR_VERSION}.")
  endif()
  set(INSTALLED_QT_VERSION ${CMake_QT_MAJOR_VERSION})
else()
  find_package(Qt6Widgets QUIET)
  set(INSTALLED_QT_VERSION 6)
  if(NOT Qt6Widgets_FOUND)
    find_package(Qt5Widgets QUIET)
    if(NOT Qt5Widgets_FOUND)
      message(FATAL_ERROR "Could not find a valid Qt installation.")
    endif()
    set(INSTALLED_QT_VERSION 5)
  endif()
endif()

find_package(Qt${INSTALLED_QT_VERSION}
    COMPONENTS ${QT_COMPONENTS}
    REQUIRED QUIET
)

set(CMake_QT_EXTRA_LIBRARIES)

# Try to find the package WinExtras for the task bar progress
if(WIN32)
  find_package(Qt${INSTALLED_QT_VERSION}WinExtras QUIET)
  if(Qt${INSTALLED_QT_VERSION}WinExtras_FOUND)
    add_compile_definitions(QT_WINEXTRAS)
    list(APPEND CMake_QT_EXTRA_LIBRARIES Qt${INSTALLED_QT_VERSION}::WinExtras)
    list(APPEND QT_COMPONENTS WinExtras)
  endif()
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt${INSTALLED_QT_VERSION}Widgets_EXECUTABLE_COMPILE_FLAGS}")

if(CMake_QT_STATIC_QXcbIntegrationPlugin_LIBRARIES)
  list(APPEND CMake_QT_EXTRA_LIBRARIES ${CMake_QT_STATIC_QXcbIntegrationPlugin_LIBRARIES})
  set_property(SOURCE CMakeSetup.cxx
    PROPERTY COMPILE_DEFINITIONS USE_QXcbIntegrationPlugin)
endif()

if(CMake_QT_STATIC_QWindowsIntegrationPlugin_LIBRARIES)
  list(APPEND CMake_QT_EXTRA_LIBRARIES ${CMake_QT_STATIC_QWindowsIntegrationPlugin_LIBRARIES})
  set_property(SOURCE CMakeSetup.cxx
    PROPERTY COMPILE_DEFINITIONS USE_QWindowsIntegrationPlugin)
endif()

# We need to install platform plugin and add qt.conf for Qt5 on Mac and Windows.
if(CMake_INSTALL_DEPENDENCIES AND (APPLE OR WIN32))
  function(_qt_get_plugin_name_with_version target out_var)
      string(REGEX REPLACE "^Qt::(.+)" "Qt${INSTALLED_QT_VERSION}::\\1"
             qt_plugin_with_version "${target}")
      if(TARGET "${qt_plugin_with_version}")
          set("${out_var}" "${qt_plugin_with_version}" PARENT_SCOPE)
      else()
          set("${out_var}" "" PARENT_SCOPE)
      endif()
  endfunction()
  macro(install_qt_plugin _qt_plugin_name _qt_plugins_var)
    if(TARGET "${_qt_plugin_name}")
      get_target_property(_qt_plugin_path "${_qt_plugin_name}" LOCATION)
    else()
      _qt_get_plugin_name_with_version("Qt::${_qt_plugin_name}" _qt_plugin_with_version_name)
      if(TARGET "${_qt_plugin_with_version_name}")
        get_target_property(_qt_plugin_path "${_qt_plugin_with_version_name}" LOCATION)
      endif()
    endif()
    if(EXISTS "${_qt_plugin_path}")
      get_filename_component(_qt_plugin_file "${_qt_plugin_path}" NAME)
      get_filename_component(_qt_plugin_type "${_qt_plugin_path}" PATH)
      get_filename_component(_qt_plugin_type "${_qt_plugin_type}" NAME)
      if(APPLE)
        set(_qt_plugin_dir "PlugIns")
      elseif(WIN32)
        set(_qt_plugin_dir "plugins")
      endif()
      set(_qt_plugin_dest "${_qt_plugin_dir}/${_qt_plugin_type}")
      install(FILES "${_qt_plugin_path}"
        DESTINATION "${_qt_plugin_dest}"
        ${COMPONENT})
      set(${_qt_plugins_var}
        "${${_qt_plugins_var}};\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${_qt_plugin_dest}/${_qt_plugin_file}")
    endif()
  endmacro()
  macro(install_qt_plugins _comps _plugins_var)
    foreach(_qt_comp IN LISTS ${_comps})
      if(INSTALLED_QT_VERSION VERSION_LESS 6)
        set(_qt_module_plugins ${Qt${INSTALLED_QT_VERSION}${_qt_comp}_PLUGINS})
      else()
        get_target_property(_qt_module_plugins Qt${INSTALLED_QT_VERSION}::${_qt_comp} QT_PLUGINS)
      endif()
      foreach(_qt_plugin IN LISTS _qt_module_plugins)
        if(INSTALLED_QT_VERSION VERSION_GREATER_EQUAL 6)
          # Qt6 provides the plugins as individual packages that need to be found.
          find_package(Qt${INSTALLED_QT_VERSION}${_qt_plugin} QUIET
            PATHS ${Qt${INSTALLED_QT_VERSION}${_qt_comp}_DIR})
        endif()
        install_qt_plugin("${_qt_plugin}" "${_plugins_var}")
      endforeach()
    endforeach()
  endmacro()
  if(APPLE)
    if(INSTALLED_QT_VERSION VERSION_EQUAL 5)
      install_qt_plugin("Qt5::QCocoaIntegrationPlugin" QT_PLUGINS)
      if(TARGET Qt5::QMacStylePlugin)
        install_qt_plugin("Qt5::QMacStylePlugin" QT_PLUGINS)
      endif()
    else()
      # FIXME: Minimize plugins for Qt6.
      install_qt_plugins(QT_COMPONENTS QT_PLUGINS)
    endif()
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/qt.conf"
      "[Paths]\nPlugins = ${_qt_plugin_dir}\n")
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/qt.conf"
      DESTINATION "${CMAKE_INSTALL_PREFIX}/Resources"
      ${COMPONENT})
  elseif(WIN32 AND NOT CMake_QT_STATIC_QWindowsIntegrationPlugin_LIBRARIES)
    if(INSTALLED_QT_VERSION VERSION_EQUAL 5)
      install_qt_plugin("Qt5::QWindowsIntegrationPlugin" QT_PLUGINS)
    else()
      # FIXME: Minimize plugins for Qt6.
      install_qt_plugins(QT_COMPONENTS QT_PLUGINS)
    endif()
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/qt.conf"
      "[Paths]\nPlugins = ../${_qt_plugin_dir}\n")
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/qt.conf"
      DESTINATION bin
      ${COMPONENT})
  endif()
endif()

get_property(_Qt_Core_LOCATION TARGET Qt${INSTALLED_QT_VERSION}::Core PROPERTY LOCATION)
get_filename_component(Qt_BIN_DIR "${_Qt_Core_LOCATION}" PATH)
if(APPLE)
  get_filename_component(Qt_BIN_DIR "${Qt_BIN_DIR}" PATH)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(
  CMakeGUILib STATIC
  AddCacheEntry.cxx
  AddCacheEntry.h
  CMakeSetupDialog.cxx
  CMakeSetupDialog.h
  Compilers.h
  EnvironmentDialog.cxx
  EnvironmentDialog.h
  FirstConfigure.cxx
  FirstConfigure.h
  QCMake.cxx
  QCMake.h
  QCMakeCacheView.cxx
  QCMakeCacheView.h
  QCMakePreset.cxx
  QCMakePreset.h
  QCMakePresetComboBox.cxx
  QCMakePresetComboBox.h
  QCMakePresetItemModel.cxx
  QCMakePresetItemModel.h
  QCMakeSizeType.h
  QCMakeWidgets.cxx
  QCMakeWidgets.h
  RegexExplorer.cxx
  RegexExplorer.h
  WarningMessagesDialog.cxx
  WarningMessagesDialog.h
  )
# CMake_QT_EXTRA_LIBRARIES have to come before the main libraries on the link line
target_link_libraries(
  CMakeGUILib
  PUBLIC
    CMakeLib
    ${CMake_QT_EXTRA_LIBRARIES}
    Qt${INSTALLED_QT_VERSION}::Core
    Qt${INSTALLED_QT_VERSION}::Widgets
  )

set(UI_SRCS
  CMakeSetupDialog.ui
  Compilers.ui
  CrossCompiler.ui
  AddCacheEntry.ui
  EnvironmentDialog.ui
  RegexExplorer.ui
  WarningMessagesDialog.ui
  )
set(MOC_SRCS
  AddCacheEntry.h
  Compilers.h
  CMakeSetupDialog.h
  EnvironmentDialog.h
  FirstConfigure.h
  QCMake.h
  QCMakeCacheView.h
  QCMakePresetComboBox.h
  QCMakePresetItemModel.h
  QCMakeWidgets.h
  RegexExplorer.h
  WarningMessagesDialog.h
  )
set(QRC_SRCS CMakeSetup.qrc)

if(INSTALLED_QT_VERSION VERSION_LESS 6)
  qt5_wrap_ui(UI_BUILT_SRCS ${UI_SRCS})
  qt5_wrap_cpp(MOC_BUILT_SRCS ${MOC_SRCS})
  qt5_add_resources(QRC_BUILT_SRCS ${QRC_SRCS})
else()
  qt_wrap_ui(UI_BUILT_SRCS ${UI_SRCS})
  qt_wrap_cpp(MOC_BUILT_SRCS ${MOC_SRCS})
  qt_add_resources(QRC_BUILT_SRCS ${QRC_SRCS})
endif()
add_library(CMakeGUIQRCLib OBJECT ${QRC_BUILT_SRCS})

if(FALSE) # CMake's bootstrap binary does not support automoc
  set(CMAKE_AUTOMOC 1)
  set(CMAKE_AUTORCC 1)
  set(CMAKE_AUTOUIC 1)
else()
  target_sources(
    CMakeGUILib
    PRIVATE
      ${UI_BUILT_SRCS}
      ${MOC_BUILT_SRCS}
  )
endif()

if(USE_LGPL)
  install(FILES ${CMake_SOURCE_DIR}/Licenses/LGPLv${USE_LGPL}.txt
    DESTINATION ${CMAKE_DATA_DIR}/Licenses
    ${COMPONENT})
  set_property(SOURCE CMakeSetupDialog.cxx
    PROPERTY COMPILE_DEFINITIONS USE_LGPL="${USE_LGPL}")
endif()

add_library(CMakeGUIMainLib STATIC CMakeSetup.cxx)
target_link_libraries(
  CMakeGUIMainLib
  PUBLIC
    CMakeGUILib
  )

add_executable(cmake-gui WIN32 MACOSX_BUNDLE CMakeGUIExec.cxx)
target_link_libraries(cmake-gui
  PRIVATE
    CMakeGUIMainLib
    CMakeGUIQRCLib
    $<TARGET_NAME_IF_EXISTS:CMakeVersion>
    ManifestLib
    Qt${INSTALLED_QT_VERSION}::Core
  )

if(WIN32)
  target_sources(CMakeGUIMainLib INTERFACE CMakeSetup.rc)
endif()
if(APPLE)
  target_sources(CMakeGUIMainLib INTERFACE CMakeSetup.icns)
  set(MACOSX_BUNDLE_ICON_FILE CMakeSetup.icns)
  set_source_files_properties(CMakeSetup.icns PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources)
endif()

if(CMake_JOB_POOL_LINK_BIN)
  set_property(TARGET cmake-gui PROPERTY JOB_POOL_LINK "link-bin")
endif()

# cmake-gui has not been updated for `include-what-you-use`.
# Block the tool until this is done.
set_target_properties(CMakeGUILib CMakeGUIMainLib cmake-gui PROPERTIES
  CXX_INCLUDE_WHAT_YOU_USE ""
  )

# Files generated by MOC, RCC, and UIC may produce clang-tidy warnings.
# We generate a dummy .clang-tidy file in the binary directory that disables
# all clang-tidy checks except one that will never match.  This one check is
# necessary; clang-tidy reports an error when no checks are enabled.
# Since the Qt code generators will generate source files in the binary tree,
# clang-tidy will load the configuration from this dummy file when the sources
# are built.
file(WRITE "${QtDialog_BINARY_DIR}/.clang-tidy" "
---
Checks: '-*,llvm-twine-local'
...
")

if(APPLE)
  file(STRINGS "${CMake_SOURCE_DIR}/Copyright.txt" copyright_line
    LIMIT_COUNT 1 REGEX "^Copyright 2000-20[0-9][0-9] Kitware")

  set_target_properties(cmake-gui PROPERTIES
    OUTPUT_NAME CMake
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${CMAKE_BUNDLE_VERSION}"
    # TBD: MACOSX_BUNDLE_BUNDLE_VERSION "${CMAKE_BUNDLE_VERSION}"
    MACOSX_BUNDLE_COPYRIGHT "${copyright_line}"
    MACOSX_BUNDLE_GUI_IDENTIFIER "org.cmake.cmake"
    )

  # Create a symlink in the build tree to provide a "cmake-gui" next
  # to the "cmake" executable that refers to the application bundle.
  add_custom_command(TARGET cmake-gui POST_BUILD
    COMMAND ln -sf CMake.app/Contents/MacOS/CMake
                   $<TARGET_FILE_DIR:cmake>/cmake-gui
    )
endif()

install(TARGETS cmake-gui
  RUNTIME DESTINATION bin ${COMPONENT}
  BUNDLE DESTINATION "${CMAKE_BUNDLE_LOCATION}" ${COMPONENT})

if(UNIX AND NOT APPLE)
  foreach(size IN ITEMS 32 128)
    install(
      FILES       "${CMAKE_CURRENT_SOURCE_DIR}/CMakeSetup${size}.png"
      DESTINATION "${CMAKE_XDGDATA_DIR}/icons/hicolor/${size}x${size}/apps"
      ${COMPONENT}
      RENAME      "CMakeSetup.png")
  endforeach()

  # install a desktop file so CMake appears in the application start menu
  # with an icon
  install(FILES cmake-gui.desktop
    DESTINATION "${CMAKE_XDGDATA_DIR}/applications"
    ${COMPONENT})
  install(FILES cmakecache.xml
    DESTINATION "${CMAKE_XDGDATA_DIR}/mime/packages"
    ${COMPONENT})
endif()

if(APPLE)
  install(CODE "
    execute_process(COMMAND ln -s \"../MacOS/CMake\" cmake-gui
        WORKING_DIRECTORY \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/bin)
  " ${COMPONENT})
endif()

if(CMake_INSTALL_DEPENDENCIES AND (APPLE OR WIN32))
  # install rules for including 3rd party libs such as Qt
  # if a system Qt is used (e.g. installed in /usr/lib/), it will not be included in the installation
  set(fixup_exe "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/bin/cmake-gui${CMAKE_EXECUTABLE_SUFFIX}")
  if(APPLE)
    set(fixup_exe "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/MacOS/CMake")
  endif()
  install(CODE "
    include(\"${CMake_SOURCE_DIR}/Modules/BundleUtilities.cmake\")
    set(BU_CHMOD_BUNDLE_ITEMS ON)
    fixup_bundle(\"${fixup_exe}\" \"${QT_PLUGINS}\" \"${Qt_BIN_DIR};${QT_LIBRARY_DIR};${QT_BINARY_DIR}\")
  " ${COMPONENT})
endif()

set(CMAKE_PACKAGE_QTGUI TRUE)
configure_file(QtDialogCPack.cmake.in QtDialogCPack.cmake @ONLY)
