# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindVulkan
----------

.. versionadded:: 3.7

Find Vulkan, which is a low-overhead, cross-platform 3D graphics
and computing API.

IMPORTED Targets
^^^^^^^^^^^^^^^^

This module defines :prop_tgt:`IMPORTED` targets if Vulkan has been found:

``Vulkan::Vulkan``
  The main Vulkan library.

``Vulkan::glslc``
  .. versionadded:: 3.19

  The GLSLC SPIR-V compiler, if it has been found.

``Vulkan::Headers``
  .. versionadded:: 3.21

  Provides just Vulkan headers include paths, if found.  No library is
  included in this target.  This can be useful for applications that
  load Vulkan library dynamically.

``Vulkan::glslangValidator``
  .. versionadded:: 3.21

  The glslangValidator tool, if found.  It is used to compile GLSL and
  HLSL shaders into SPIR-V.

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

``Vulkan_FOUND``
  set to true if Vulkan was found
``Vulkan_INCLUDE_DIRS``
  include directories for Vulkan
``Vulkan_LIBRARIES``
  link against this library to use Vulkan
``Vulkan_VERSION``
  .. versionadded:: 3.23

  value from ``vulkan/vulkan_core.h``

The module will also defines these cache variables:

``Vulkan_INCLUDE_DIR``
  the Vulkan include directory
``Vulkan_LIBRARY``
  the path to the Vulkan library
``Vulkan_GLSLC_EXECUTABLE``
  the path to the GLSL SPIR-V compiler
``Vulkan_GLSLANG_VALIDATOR_EXECUTABLE``
  the path to the glslangValidator tool

Hints
^^^^^

.. versionadded:: 3.18

The ``VULKAN_SDK`` environment variable optionally specifies the
location of the Vulkan SDK root directory for the given
architecture. It is typically set by sourcing the toplevel
``setup-env.sh`` script of the Vulkan SDK directory into the shell
environment.

#]=======================================================================]

if(WIN32)
  find_path(Vulkan_INCLUDE_DIR
    NAMES vulkan/vulkan.h
    HINTS
      "$ENV{VULKAN_SDK}/Include"
    )

  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    find_library(Vulkan_LIBRARY
      NAMES vulkan-1
      HINTS
        "$ENV{VULKAN_SDK}/Lib"
        "$ENV{VULKAN_SDK}/Bin"
      )
    find_program(Vulkan_GLSLC_EXECUTABLE
      NAMES glslc
      HINTS
        "$ENV{VULKAN_SDK}/Bin"
      )
    find_program(Vulkan_GLSLANG_VALIDATOR_EXECUTABLE
      NAMES glslangValidator
      HINTS
        "$ENV{VULKAN_SDK}/Bin"
      )
  elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    find_library(Vulkan_LIBRARY
      NAMES vulkan-1
      HINTS
        "$ENV{VULKAN_SDK}/Lib32"
        "$ENV{VULKAN_SDK}/Bin32"
      )
    find_program(Vulkan_GLSLC_EXECUTABLE
      NAMES glslc
      HINTS
        "$ENV{VULKAN_SDK}/Bin32"
      )
    find_program(Vulkan_GLSLANG_VALIDATOR_EXECUTABLE
      NAMES glslangValidator
      HINTS
        "$ENV{VULKAN_SDK}/Bin32"
      )
  endif()
else()
  find_path(Vulkan_INCLUDE_DIR
    NAMES vulkan/vulkan.h
    HINTS "$ENV{VULKAN_SDK}/include")
  find_library(Vulkan_LIBRARY
    NAMES vulkan
    HINTS "$ENV{VULKAN_SDK}/lib")
  find_program(Vulkan_GLSLC_EXECUTABLE
    NAMES glslc
    HINTS "$ENV{VULKAN_SDK}/bin")
  find_program(Vulkan_GLSLANG_VALIDATOR_EXECUTABLE
    NAMES glslangValidator
    HINTS "$ENV{VULKAN_SDK}/bin")
endif()

set(Vulkan_LIBRARIES ${Vulkan_LIBRARY})
set(Vulkan_INCLUDE_DIRS ${Vulkan_INCLUDE_DIR})

# detect version e.g 1.2.189
set(Vulkan_VERSION "")
if(Vulkan_INCLUDE_DIR)
  set(VULKAN_CORE_H ${Vulkan_INCLUDE_DIR}/vulkan/vulkan_core.h)
  if(EXISTS ${VULKAN_CORE_H})
    file(STRINGS  ${VULKAN_CORE_H} VulkanHeaderVersionLine REGEX "^#define VK_HEADER_VERSION ")
    string(REGEX MATCHALL "[0-9]+" VulkanHeaderVersion "${VulkanHeaderVersionLine}")
    file(STRINGS  ${VULKAN_CORE_H} VulkanHeaderVersionLine2 REGEX "^#define VK_HEADER_VERSION_COMPLETE ")
    string(REGEX MATCHALL "[0-9]+" VulkanHeaderVersion2 "${VulkanHeaderVersionLine2}")
    list(LENGTH VulkanHeaderVersion2 _len)
    #  versions >= 1.2.175 have an additional numbers in front of e.g. '0, 1, 2' instead of '1, 2'
    if(_len EQUAL 3)
        list(REMOVE_AT VulkanHeaderVersion2 0)
    endif()
    list(APPEND VulkanHeaderVersion2 ${VulkanHeaderVersion})
    list(JOIN VulkanHeaderVersion2 "." Vulkan_VERSION)
  endif()
endif()

include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
find_package_handle_standard_args(Vulkan
  REQUIRED_VARS
    Vulkan_LIBRARY
    Vulkan_INCLUDE_DIR
  VERSION_VAR
    Vulkan_VERSION
)

mark_as_advanced(Vulkan_INCLUDE_DIR Vulkan_LIBRARY Vulkan_GLSLC_EXECUTABLE
                 Vulkan_GLSLANG_VALIDATOR_EXECUTABLE)

if(Vulkan_FOUND AND NOT TARGET Vulkan::Vulkan)
  add_library(Vulkan::Vulkan UNKNOWN IMPORTED)
  set_target_properties(Vulkan::Vulkan PROPERTIES
    IMPORTED_LOCATION "${Vulkan_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${Vulkan_INCLUDE_DIRS}")
endif()

if(Vulkan_FOUND AND NOT TARGET Vulkan::Headers)
  add_library(Vulkan::Headers INTERFACE IMPORTED)
  set_target_properties(Vulkan::Headers PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${Vulkan_INCLUDE_DIRS}")
endif()

if(Vulkan_FOUND AND Vulkan_GLSLC_EXECUTABLE AND NOT TARGET Vulkan::glslc)
  add_executable(Vulkan::glslc IMPORTED)
  set_property(TARGET Vulkan::glslc PROPERTY IMPORTED_LOCATION "${Vulkan_GLSLC_EXECUTABLE}")
endif()

if(Vulkan_FOUND AND Vulkan_GLSLANG_VALIDATOR_EXECUTABLE AND NOT TARGET Vulkan::glslangValidator)
  add_executable(Vulkan::glslangValidator IMPORTED)
  set_property(TARGET Vulkan::glslangValidator PROPERTY IMPORTED_LOCATION "${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE}")
endif()
