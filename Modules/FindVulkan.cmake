# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindVulkan
----------

.. versionadded:: 3.7

Find Vulkan, which is a low-overhead, cross-platform 3D graphics
and computing API.

Optional COMPONENTS
^^^^^^^^^^^^^^^^^^^

.. versionadded:: 3.24

This module respects several optional COMPONENTS: ``shaderc_combined``.  There
are corresponding import targets for each of these flags.

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

``Vulkan::shaderc_combined``
  .. versionadded:: 3.24

  Defined if SDK has the Google static library for Vulkan shader compilation
  (shaderc_combined).

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
``Vulkan_shaderc_combined_FOUND``
  .. versionadded:: 3.24

  True, if the SDK has the shaderc_combined library.

.. versionadded:: 3.24
  Variables for component library ``shaderc_combined``.

The module will also defines these cache variables:

``Vulkan_INCLUDE_DIR``
  the Vulkan include directory
``Vulkan_LIBRARY``
  the path to the Vulkan library
``Vulkan_GLSLC_EXECUTABLE``
  the path to the GLSL SPIR-V compiler
``Vulkan_GLSLANG_VALIDATOR_EXECUTABLE``
  the path to the glslangValidator tool
``Vulkan_shaderc_combined_LIBRARY``
  .. versionadded:: 3.24

  Path to the shaderc_combined library.

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
  set(_Vulkan_library_name vulkan-1)
  set(_Vulkan_hint_include_search_paths
    "$ENV{VULKAN_SDK}/Include"
  )
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(_Vulkan_hint_executable_search_paths
      "$ENV{VULKAN_SDK}/Bin"
    )
    set(_Vulkan_hint_library_search_paths
      "$ENV{VULKAN_SDK}/Lib"
      "$ENV{VULKAN_SDK}/Bin"
    )
  else()
    set(_Vulkan_hint_executable_search_paths
      "$ENV{VULKAN_SDK}/Bin32"
    )
    set(_Vulkan_hint_library_search_paths
      "$ENV{VULKAN_SDK}/Lib32"
      "$ENV{VULKAN_SDK}/Bin32"
    )
  endif()
else()
  set(_Vulkan_library_name vulkan)
  set(_Vulkan_hint_include_search_paths
    "$ENV{VULKAN_SDK}/include"
  )
  set(_Vulkan_hint_executable_search_paths
    "$ENV{VULKAN_SDK}/bin"
  )
  set(_Vulkan_hint_library_search_paths
    "$ENV{VULKAN_SDK}/lib"
  )
endif()

find_path(Vulkan_INCLUDE_DIR
  NAMES vulkan/vulkan.h
  HINTS
    ${_Vulkan_hint_include_search_paths}
  )
mark_as_advanced(Vulkan_INCLUDE_DIR)

find_library(Vulkan_LIBRARY
  NAMES ${_Vulkan_library_name}
  HINTS
    ${_Vulkan_hint_library_search_paths}
  )
mark_as_advanced(Vulkan_LIBRARY)

find_program(Vulkan_GLSLC_EXECUTABLE
  NAMES glslc
  HINTS
    ${_Vulkan_hint_executable_search_paths}
  )
mark_as_advanced(Vulkan_GLSLC_EXECUTABLE)

find_program(Vulkan_GLSLANG_VALIDATOR_EXECUTABLE
  NAMES glslangValidator
  HINTS
    ${_Vulkan_hint_executable_search_paths}
  )
mark_as_advanced(Vulkan_GLSLANG_VALIDATOR_EXECUTABLE)

if(shaderc_combined IN_LIST Vulkan_FIND_COMPONENTS)
  find_library(Vulkan_shaderc_combined_LIBRARY
    NAMES shaderc_combined
    HINTS
    ${_Vulkan_hint_library_search_paths})
  mark_as_advanced(Vulkan_shaderc_combined_LIBRARY)

  find_library(Vulkan_shaderc_combined_DEBUG_LIBRARY
    NAMES shaderc_combinedd
    HINTS
    ${_Vulkan_hint_library_search_paths})
  mark_as_advanced(Vulkan_shaderc_combined_DEBUG_LIBRARY)
endif()

function(_Vulkan_set_library_component_found component)
  if(Vulkan_${component}_LIBRARY OR Vulkan_${component}_DEBUG_LIBRARY)
    set(Vulkan_${component}_FOUND TRUE PARENT_SCOPE)

    # For Windows Vulkan SDK, third party tools binaries are provided with different MSVC ABI:
    #   - Release binaries uses a runtime library
    #   - Debug binaries uses a debug runtime library
    # This lead to incompatibilities in linking for some configuration types due to CMake-default or project-configured selected MSVC ABI.
    if(WIN32)
      if(NOT Vulkan_${component}_LIBRARY)
        message(WARNING
"Library ${component} for Release configuration is missing, imported target Vulkan::${component} may not be able to link when targeting this build configuration due to incompatible MSVC ABI.")
      endif()
      if(NOT Vulkan_${component}_DEBUG_LIBRARY)
        message(WARNING
"Library ${component} for Debug configuration is missing, imported target Vulkan::${component} may not be able to link when targeting this build configuration due to incompatible MSVC ABI. Consider re-installing the Vulkan SDK and request debug libraries to fix this warning.")
      endif()
    endif()
  else()
    set(Vulkan_${component}_FOUND FALSE PARENT_SCOPE)
  endif()
endfunction()

_Vulkan_set_library_component_found(shaderc_combined)

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
  HANDLE_COMPONENTS
)

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

if(Vulkan_FOUND)
  if((Vulkan_shaderc_combined_LIBRARY OR Vulkan_shaderc_combined_DEBUG_LIBRARY) AND NOT TARGET Vulkan::shaderc_combined)
    add_library(Vulkan::shaderc_combined STATIC IMPORTED)
    set_property(TARGET Vulkan::shaderc_combined
      PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES "${Vulkan_INCLUDE_DIRS}")
    if(Vulkan_shaderc_combined_LIBRARY)
      set_property(TARGET Vulkan::shaderc_combined APPEND
        PROPERTY
          IMPORTED_CONFIGURATIONS Release)
      set_property(TARGET Vulkan::shaderc_combined
        PROPERTY
          IMPORTED_LOCATION_RELEASE "${Vulkan_shaderc_combined_LIBRARY}")
    endif()
    if(Vulkan_shaderc_combined_DEBUG_LIBRARY)
      set_property(TARGET Vulkan::shaderc_combined APPEND
        PROPERTY
          IMPORTED_CONFIGURATIONS Debug)
      set_property(TARGET Vulkan::shaderc_combined
        PROPERTY
          IMPORTED_LOCATION_DEBUG "${Vulkan_shaderc_combined_DEBUG_LIBRARY}")
    endif()

    if(UNIX)
      find_package(Threads REQUIRED)
      target_link_libraries(Vulkan::shaderc_combined
        INTERFACE
          Threads::Threads)
    endif()
  endif()
endif()

unset(_Vulkan_library_name)
unset(_Vulkan_hint_include_search_paths)
unset(_Vulkan_hint_executable_search_paths)
unset(_Vulkan_hint_library_search_paths)
