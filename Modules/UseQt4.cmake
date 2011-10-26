# - Use Module for QT4
# Sets up C and C++ to use Qt 4.  It is assumed that FindQt.cmake
# has already been loaded.  See FindQt.cmake for information on
# how to load Qt 4 into your CMake project.

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

add_definitions(${QT_DEFINITIONS})
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS_DEBUG QT_DEBUG)
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS_RELEASE QT_NO_DEBUG)
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS_RELWITHDEBINFO QT_NO_DEBUG)
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS_MINSIZEREL QT_NO_DEBUG)
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS QT_NO_DEBUG)
endif()

include_directories(${QT_INCLUDE_DIR})

set(QT_LIBRARIES "")
set(QT_LIBRARIES_PLUGINS "")

if(QT_USE_QTMAIN)
  if(Q_WS_WIN)
    set(QT_LIBRARIES ${QT_LIBRARIES} ${QT_QTMAIN_LIBRARY})
  endif(Q_WS_WIN)
endif(QT_USE_QTMAIN)

if(QT_DONT_USE_QTGUI)
  set(QT_USE_QTGUI 0)
else(QT_DONT_USE_QTGUI)
  set(QT_USE_QTGUI 1)
endif(QT_DONT_USE_QTGUI)

if(QT_DONT_USE_QTCORE)
  set(QT_USE_QTCORE 0)
else(QT_DONT_USE_QTCORE)
  set(QT_USE_QTCORE 1)
endif(QT_DONT_USE_QTCORE)

if(QT_USE_QT3SUPPORT)
  add_definitions(-DQT3_SUPPORT)
endif(QT_USE_QT3SUPPORT)

# list dependent modules, so dependent libraries are added
set(QT_QT3SUPPORT_MODULE_DEPENDS QTGUI QTSQL QTXML QTNETWORK QTCORE)
set(QT_QTSVG_MODULE_DEPENDS QTGUI QTXML QTCORE)
set(QT_QTUITOOLS_MODULE_DEPENDS QTGUI QTXML QTCORE)
set(QT_QTHELP_MODULE_DEPENDS QTGUI QTSQL QTXML QTNETWORK QTCORE)
if(QT_QTDBUS_FOUND)
  set(QT_PHONON_MODULE_DEPENDS QTGUI QTDBUS QTCORE)
else(QT_QTDBUS_FOUND)
  set(QT_PHONON_MODULE_DEPENDS QTGUI QTCORE)
endif(QT_QTDBUS_FOUND)
set(QT_QTDBUS_MODULE_DEPENDS QTXML QTCORE)
set(QT_QTXMLPATTERNS_MODULE_DEPENDS QTNETWORK QTCORE)
set(QT_QAXCONTAINER_MODULE_DEPENDS QTGUI QTCORE)
set(QT_QAXSERVER_MODULE_DEPENDS QTGUI QTCORE)
set(QT_QTSCRIPTTOOLS_MODULE_DEPENDS QTGUI QTCORE)
set(QT_QTWEBKIT_MODULE_DEPENDS QTXMLPATTERNS QTGUI QTCORE)
set(QT_QTDECLARATIVE_MODULE_DEPENDS QTWEBKIT QTSCRIPT QTSVG QTSQL QTXMLPATTERNS QTXML QTOPENGL QTGUI QTNETWORK QTCORE)
set(QT_QTMULTIMEDIA_MODULE_DEPENDS QTGUI QTCORE)
set(QT_QTOPENGL_MODULE_DEPENDS QTGUI QTCORE)
set(QT_QTSCRIPT_MODULE_DEPENDS QTCORE)
set(QT_QTGUI_MODULE_DEPENDS QTCORE)
set(QT_QTTEST_MODULE_DEPENDS QTCORE)
set(QT_QTXML_MODULE_DEPENDS QTCORE)
set(QT_QTSQL_MODULE_DEPENDS QTCORE)
set(QT_QTNETWORK_MODULE_DEPENDS QTCORE)

# Qt modules  (in order of dependence)
foreach(module QT3SUPPORT QTOPENGL QTASSISTANT QTDESIGNER QTMOTIF QTNSPLUGIN
               QAXSERVER QAXCONTAINER QTDECLARATIVE QTSCRIPT QTSVG QTUITOOLS QTHELP
               QTWEBKIT PHONON QTSCRIPTTOOLS QTMULTIMEDIA QTGUI QTTEST QTDBUS QTXML QTSQL
               QTXMLPATTERNS QTNETWORK QTCORE)

  if(QT_USE_${module} OR QT_USE_${module}_DEPENDS)
    if(QT_${module}_FOUND)
      if(QT_USE_${module})
        string(REPLACE "QT" "" qt_module_def "${module}")
        add_definitions(-DQT_${qt_module_def}_LIB)
        include_directories(${QT_${module}_INCLUDE_DIR})
      endif(QT_USE_${module})
      set(QT_LIBRARIES ${QT_LIBRARIES} ${QT_${module}_LIBRARY})
      set(QT_LIBRARIES_PLUGINS ${QT_LIBRARIES_PLUGINS} ${QT_${module}_PLUGINS})
      if(QT_IS_STATIC)
        set(QT_LIBRARIES ${QT_LIBRARIES} ${QT_${module}_LIB_DEPENDENCIES})
      endif(QT_IS_STATIC)
      foreach(depend_module ${QT_${module}_MODULE_DEPENDS})
        set(QT_USE_${depend_module}_DEPENDS 1)
      endforeach(depend_module ${QT_${module}_MODULE_DEPENDS})
    else(QT_${module}_FOUND)
      message("Qt ${module} library not found.")
    endif(QT_${module}_FOUND)
  endif(QT_USE_${module} OR QT_USE_${module}_DEPENDS)

endforeach(module)

