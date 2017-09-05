cmake_minimum_required(VERSION 3.9)
cmake_policy(SET CMP0071 NEW)
project(QtAutogenRerun)

# Tell find_package(Qt5) where to find Qt.
if(QT_QMAKE_EXECUTABLE)
  get_filename_component(Qt_BIN_DIR "${QT_QMAKE_EXECUTABLE}" PATH)
  get_filename_component(Qt_PREFIX_DIR "${Qt_BIN_DIR}" PATH)
  set(CMAKE_PREFIX_PATH ${Qt_PREFIX_DIR})
endif()

if (QT_TEST_VERSION STREQUAL 4)
  find_package(Qt4 REQUIRED)

  # Include this directory before using the UseQt4 file.
  add_subdirectory(defines_test)

  include(UseQt4)

  set(QT_QTCORE_TARGET Qt4::QtCore)

else()
  if (NOT QT_TEST_VERSION STREQUAL 5)
    message(SEND_ERROR "Invalid Qt version specified.")
  endif()
  find_package(Qt5Widgets REQUIRED)

  set(QT_QTCORE_TARGET Qt5::Core)

  include_directories(${Qt5Widgets_INCLUDE_DIRS})
  set(QT_LIBRARIES Qt5::Widgets)

  if(Qt5_POSITION_INDEPENDENT_CODE AND CMAKE_CXX_COMPILE_OPTIONS_PIC)
    add_definitions(${CMAKE_CXX_COMPILE_OPTIONS_PIC})
  endif()

endif()

# Dummy executable to generate clean target
add_executable(dummy dummy.cpp)

# -- Test
include("mocRerun.cmake")

# -- Test
# Tests Q_PLUGIN_METADATA json file change detection
if (NOT QT_TEST_VERSION STREQUAL 4)
  include("mocPlugin.cmake")
endif()

# -- Test
include("rccDepends.cmake")
