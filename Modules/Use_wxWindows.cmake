# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
Use_wxWindows
-------------

.. deprecated:: 2.8.10

  Use :module:`find_package(wxWidgets) <FindwxWidgets>` instead.

This convenience include finds if wxWindows library is installed and sets the
appropriate libraries, include directories, flags, etc.

Examples
^^^^^^^^

Include ``Use_wxWindows`` module in project's ``CMakeLists.txt``:

.. code-block:: cmake

  # CMakeLists.txt
  include(Use_wxWindows)

When the GL support is required, set ``WXWINDOWS_USE_GL`` *before* including
this module:

.. code-block:: cmake

  set(WXWINDOWS_USE_GL ON)
  include(Use_wxWindows)
#]=======================================================================]

# Author: Jan Woetzel <jw -at- mip.informatik.uni-kiel.de> (07/2003)

# -----------------------------------------------------
# 16.Feb.2004: changed INCLUDE to FIND_PACKAGE to read from users own non-system CMAKE_MODULE_PATH (Jan Woetzel JW)
# 07/2006: rewrite as FindwxWidgets.cmake, kept for backward compatibility JW

message(
  DEPRECATION
  "Use_wxWindows module is DEPRECATED.\n"
  "Please use find_package(wxWidgets) instead. (JW)"
)

# ------------------------

find_package( wxWindows )

if(WXWINDOWS_FOUND)

#message("DBG Use_wxWindows.cmake:  WXWINDOWS_INCLUDE_DIR=${WXWINDOWS_INCLUDE_DIR} WXWINDOWS_LINK_DIRECTORIES=${WXWINDOWS_LINK_DIRECTORIES}     WXWINDOWS_LIBRARIES=${WXWINDOWS_LIBRARIES}  CMAKE_WXWINDOWS_CXX_FLAGS=${CMAKE_WXWINDOWS_CXX_FLAGS} WXWINDOWS_DEFINITIONS=${WXWINDOWS_DEFINITIONS}")

 if(WXWINDOWS_INCLUDE_DIR)
    include_directories(${WXWINDOWS_INCLUDE_DIR})
  endif()
 if(WXWINDOWS_LINK_DIRECTORIES)
    link_directories(${WXWINDOWS_LINK_DIRECTORIES})
  endif()
  if(WXWINDOWS_LIBRARIES)
    link_libraries(${WXWINDOWS_LIBRARIES})
  endif()
  if (CMAKE_WXWINDOWS_CXX_FLAGS)
    string(APPEND CMAKE_CXX_FLAGS " ${CMAKE_WXWINDOWS_CXX_FLAGS}")
  endif()
  if(WXWINDOWS_DEFINITIONS)
    add_definitions(${WXWINDOWS_DEFINITIONS})
  endif()
else()
  message(SEND_ERROR "wxWindows not found by Use_wxWindows.cmake")
endif()
