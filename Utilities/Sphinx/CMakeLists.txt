#=============================================================================
# CMake - Cross Platform Makefile Generator
# Copyright 2000-2013 Kitware, Inc., Insight Software Consortium
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
if(NOT CMake_SOURCE_DIR)
  set(CMakeHelp_STANDALONE 1)
  cmake_minimum_required(VERSION 2.8.2 FATAL_ERROR)
  set(CMAKE_LEGACY_CYGWIN_WIN32 0) # Remove when CMake >= 2.8.4 is required
  get_filename_component(tmp "${CMAKE_CURRENT_SOURCE_DIR}" PATH)
  get_filename_component(CMake_SOURCE_DIR "${tmp}" PATH)
  include(${CMake_SOURCE_DIR}/Source/CMakeVersionCompute.cmake)
  include(${CMake_SOURCE_DIR}/Source/CMakeInstallDestinations.cmake)
  unset(CMAKE_DATA_DIR)
  unset(CMAKE_DATA_DIR CACHE)
endif()
project(CMakeHelp NONE)

option(SPHINX_MAN "Build man pages with Sphinx" OFF)
option(SPHINX_HTML "Build html help with Sphinx" OFF)
find_program(SPHINX_EXECUTABLE
  NAMES sphinx-build
  DOC "Sphinx Documentation Builder (sphinx-doc.org)"
  )

if(NOT SPHINX_MAN AND NOT SPHINX_HTML)
  return()
elseif(NOT SPHINX_EXECUTABLE)
  message(FATAL_ERROR "SPHINX_EXECUTABLE (sphinx-build) is not found!")
endif()

set(conf_docs "${CMake_SOURCE_DIR}/Help")
set(conf_path "${CMAKE_CURRENT_SOURCE_DIR}")
set(conf_copyright "2000-2013 Kitware, Inc.")
set(conf_version "${CMake_MAJOR_VERSION}.${CMake_MINOR_VERSION}.${CMake_PATCH_VERSION}")
set(conf_release "${CMake_VERSION}")
configure_file(conf.py.in conf.py @ONLY)

set(doc_formats "")
if(SPHINX_HTML)
  list(APPEND doc_formats html)
endif()
if(SPHINX_MAN)
  list(APPEND doc_formats man)
endif()

set(doc_format_outputs "")
set(doc_format_last "")
foreach(format ${doc_formats})
  set(doc_format_output "doc_format_${format}")
  set(doc_format_log "build-${format}.log")
  add_custom_command(
    OUTPUT ${doc_format_output}
    COMMAND ${SPHINX_EXECUTABLE}
            -c ${CMAKE_CURRENT_BINARY_DIR}
            -d ${CMAKE_CURRENT_BINARY_DIR}/doctrees
            -b ${format}
            ${CMake_SOURCE_DIR}/Help
            ${CMAKE_CURRENT_BINARY_DIR}/${format}
            > ${doc_format_log} # log stdout, pass stderr
    DEPENDS ${doc_format_last}
    COMMENT "sphinx-build ${format}: see Utilities/Sphinx/${doc_format_log}"
    VERBATIM
    )
  set_property(SOURCE ${doc_format_output} PROPERTY SYMBOLIC 1)
  list(APPEND doc_format_outputs ${doc_format_output})
  set(doc_format_last ${doc_format_output})
endforeach()

add_custom_target(documentation ALL DEPENDS ${doc_format_outputs})

foreach(t
    cmake
    ccmake
    cmake-gui
    cpack
    ctest
    )
  if(TARGET ${t})
    # Build documentation after main executables.
    add_dependencies(documentation ${t})
  endif()
endforeach()

if(SPHINX_MAN)
  file(GLOB man_rst RELATIVE ${CMake_SOURCE_DIR}/Help/manual
    ${CMake_SOURCE_DIR}/Help/manual/*.[1-9].rst)
  foreach(m ${man_rst})
    if("x${m}" MATCHES "^x(.+)\\.([1-9])\\.rst$")
      set(name "${CMAKE_MATCH_1}")
      set(sec "${CMAKE_MATCH_2}")
      install(FILES ${CMAKE_CURRENT_BINARY_DIR}/man/${name}.${sec}
              DESTINATION ${CMAKE_MAN_DIR}/man${sec})
    endif()
  endforeach()
endif()

if(SPHINX_HTML)
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
          DESTINATION ${CMAKE_DOC_DIR})
endif()
