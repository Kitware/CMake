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

This module respects several optional COMPONENTS.
There are corresponding imported targets for each of these.

``glslc``
  The SPIR-V compiler.

``glslangValidator``
  The ``glslangValidator`` tool.

``glslang``
  The SPIR-V generator library.

``shaderc_combined``
  The static library for Vulkan shader compilation.

``SPIRV-Tools``
  Tools to process SPIR-V modules.

``MoltenVK``
  On macOS, an additional component ``MoltenVK`` is available.

``dxc``
  .. versionadded:: 3.25

  The DirectX Shader Compiler.

The ``glslc`` and ``glslangValidator`` components are provided even
if not explicitly requested (for backward compatibility).

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

``Vulkan::glslang``
  .. versionadded:: 3.24

  Defined if SDK has the Khronos-reference front-end shader parser and SPIR-V
  generator library (glslang).

``Vulkan::shaderc_combined``
  .. versionadded:: 3.24

  Defined if SDK has the Google static library for Vulkan shader compilation
  (shaderc_combined).

``Vulkan::SPIRV-Tools``
  .. versionadded:: 3.24

  Defined if SDK has the Khronos library to process SPIR-V modules
  (SPIRV-Tools).

``Vulkan::MoltenVK``
  .. versionadded:: 3.24

  Defined if SDK has the Khronos library which implement a subset of Vulkan API
  over Apple Metal graphics framework. (MoltenVK).

``Vulkan::volk``
  .. versionadded:: 3.25

  Defined if SDK has the Vulkan meta-loader (volk).

``Vulkan::dxc_lib``
  .. versionadded:: 3.25

  Defined if SDK has the DirectX shader compiler library.

``Vulkan::dxc_exe``
  .. versionadded:: 3.25

  Defined if SDK has the DirectX shader compiler CLI tool.

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
``Vulkan_glslc_FOUND``
  .. versionadded:: 3.24

  True, if the SDK has the glslc executable.
``Vulkan_glslangValidator_FOUND``
  .. versionadded:: 3.24

  True, if the SDK has the glslangValidator executable.
``Vulkan_glslang_FOUND``
  .. versionadded:: 3.24

  True, if the SDK has the glslang library.
``Vulkan_shaderc_combined_FOUND``
  .. versionadded:: 3.24

  True, if the SDK has the shaderc_combined library.
``Vulkan_SPIRV-Tools_FOUND``
  .. versionadded:: 3.24

  True, if the SDK has the SPIRV-Tools library.
``Vulkan_MoltenVK_FOUND``
  .. versionadded:: 3.24

  True, if the SDK has the MoltenVK library.
``Vulkan_volk_FOUND``
  .. versionadded:: 3.25

  True, if the SDK has the volk library.

``Vulkan_dxc_lib_FOUND``
  .. versionadded:: 3.25

  True, if the SDK has the DirectX shader compiler library.

``Vulkan_dxc_exe_FOUND``
  .. versionadded:: 3.25

  True, if the SDK has the DirectX shader compiler CLI tool.


The module will also defines these cache variables:

``Vulkan_INCLUDE_DIR``
  the Vulkan include directory
``Vulkan_LIBRARY``
  the path to the Vulkan library
``Vulkan_GLSLC_EXECUTABLE``
  the path to the GLSL SPIR-V compiler
``Vulkan_GLSLANG_VALIDATOR_EXECUTABLE``
  the path to the glslangValidator tool
``Vulkan_glslang_LIBRARY``
  .. versionadded:: 3.24

  Path to the glslang library.
``Vulkan_shaderc_combined_LIBRARY``
  .. versionadded:: 3.24

  Path to the shaderc_combined library.
``Vulkan_SPIRV-Tools_LIBRARY``
  .. versionadded:: 3.24

  Path to the SPIRV-Tools library.
``Vulkan_MoltenVK_LIBRARY``
  .. versionadded:: 3.24

  Path to the MoltenVK library.

``Vulkan_volk_LIBRARY``
  .. versionadded:: 3.25

  Path to the volk library.

``Vulkan_dxc_LIBRARY``
  .. versionadded:: 3.25

  Path to the DirectX shader compiler library.

``Vulkan_dxc_EXECUTABLE``
  .. versionadded:: 3.25

  Path to the DirectX shader compiler CLI tool.

Hints
^^^^^

.. versionadded:: 3.18

The ``VULKAN_SDK`` environment variable optionally specifies the
location of the Vulkan SDK root directory for the given
architecture. It is typically set by sourcing the toplevel
``setup-env.sh`` script of the Vulkan SDK directory into the shell
environment.

#]=======================================================================]

cmake_policy(PUSH)
cmake_policy(SET CMP0057 NEW)
cmake_policy(SET CMP0159 NEW) # file(STRINGS) with REGEX updates CMAKE_MATCH_<n>

# Provide compatibility with a common invalid component request that
# was silently ignored prior to CMake 3.24.
if("FATAL_ERROR" IN_LIST Vulkan_FIND_COMPONENTS)
  message(AUTHOR_WARNING
    "Ignoring unknown component 'FATAL_ERROR'.\n"
    "The find_package() command documents no such argument."
    )
  list(REMOVE_ITEM Vulkan_FIND_COMPONENTS "FATAL_ERROR")
endif()

# For backward compatibility as `FindVulkan` in previous CMake versions allow to retrieve `glslc`
# and `glslangValidator` without requesting the corresponding component.
if(NOT glslc IN_LIST Vulkan_FIND_COMPONENTS)
  list(APPEND Vulkan_FIND_COMPONENTS glslc)
endif()
if(NOT glslangValidator IN_LIST Vulkan_FIND_COMPONENTS)
  list(APPEND Vulkan_FIND_COMPONENTS glslangValidator)
endif()

if(WIN32)
  set(_Vulkan_library_name vulkan-1)
  set(_Vulkan_hint_include_search_paths
    "$ENV{VULKAN_SDK}/include"
  )
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(_Vulkan_hint_executable_search_paths
      "$ENV{VULKAN_SDK}/bin"
    )
    set(_Vulkan_hint_library_search_paths
      "$ENV{VULKAN_SDK}/lib"
      "$ENV{VULKAN_SDK}/bin"
    )
  else()
    set(_Vulkan_hint_executable_search_paths
      "$ENV{VULKAN_SDK}/bin32"
      "$ENV{VULKAN_SDK}/bin"
    )
    set(_Vulkan_hint_library_search_paths
      "$ENV{VULKAN_SDK}/lib32"
      "$ENV{VULKAN_SDK}/bin32"
      "$ENV{VULKAN_SDK}/lib"
      "$ENV{VULKAN_SDK}/bin"
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
if(APPLE AND DEFINED ENV{VULKAN_SDK})
  cmake_path(SET _MoltenVK_path NORMALIZE "$ENV{VULKAN_SDK}/../MoltenVK")
  if(EXISTS "${_MoltenVK_path}")
    list(APPEND _Vulkan_hint_include_search_paths
      "${_MoltenVK_path}/include"
    )
    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
      list(APPEND _Vulkan_hint_library_search_paths
        "${_MoltenVK_path}/dylib/iOS"
      )
    elseif(CMAKE_SYSTEM_NAME STREQUAL "tvOS")
      list(APPEND _Vulkan_hint_library_search_paths
        "${_MoltenVK_path}/dylib/tvOS"
      )
    else()
      list(APPEND _Vulkan_hint_library_search_paths
        "${_MoltenVK_path}/dylib/macOS"
      )
    endif()
  endif()
  unset(_MoltenVK_path)
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

if(glslc IN_LIST Vulkan_FIND_COMPONENTS)
  find_program(Vulkan_GLSLC_EXECUTABLE
    NAMES glslc
    HINTS
      ${_Vulkan_hint_executable_search_paths}
    )
  mark_as_advanced(Vulkan_GLSLC_EXECUTABLE)
endif()
if(glslangValidator IN_LIST Vulkan_FIND_COMPONENTS)
  find_program(Vulkan_GLSLANG_VALIDATOR_EXECUTABLE
    NAMES glslangValidator
    HINTS
      ${_Vulkan_hint_executable_search_paths}
    )
  mark_as_advanced(Vulkan_GLSLANG_VALIDATOR_EXECUTABLE)
endif()
if(glslang IN_LIST Vulkan_FIND_COMPONENTS)
  find_library(Vulkan_glslang-spirv_LIBRARY
    NAMES SPIRV
    HINTS
      ${_Vulkan_hint_library_search_paths}
    )
  mark_as_advanced(Vulkan_glslang-spirv_LIBRARY)

  find_library(Vulkan_glslang-spirv_DEBUG_LIBRARY
    NAMES SPIRVd
    HINTS
      ${_Vulkan_hint_library_search_paths}
    )
  mark_as_advanced(Vulkan_glslang-spirv_DEBUG_LIBRARY)

  find_library(Vulkan_glslang-oglcompiler_LIBRARY
    NAMES OGLCompiler
    HINTS
      ${_Vulkan_hint_library_search_paths}
    )
  mark_as_advanced(Vulkan_glslang-oglcompiler_LIBRARY)

  find_library(Vulkan_glslang-oglcompiler_DEBUG_LIBRARY
    NAMES OGLCompilerd
    HINTS
      ${_Vulkan_hint_library_search_paths}
    )
  mark_as_advanced(Vulkan_glslang-oglcompiler_DEBUG_LIBRARY)

  find_library(Vulkan_glslang-osdependent_LIBRARY
    NAMES OSDependent
    HINTS
      ${_Vulkan_hint_library_search_paths}
    )
  mark_as_advanced(Vulkan_glslang-osdependent_LIBRARY)

  find_library(Vulkan_glslang-osdependent_DEBUG_LIBRARY
    NAMES OSDependentd
    HINTS
      ${_Vulkan_hint_library_search_paths}
    )
  mark_as_advanced(Vulkan_glslang-osdependent_DEBUG_LIBRARY)

  find_library(Vulkan_glslang-machineindependent_LIBRARY
    NAMES MachineIndependent
    HINTS
      ${_Vulkan_hint_library_search_paths}
    )
  mark_as_advanced(Vulkan_glslang-machineindependent_LIBRARY)

  find_library(Vulkan_glslang-machineindependent_DEBUG_LIBRARY
    NAMES MachineIndependentd
    HINTS
      ${_Vulkan_hint_library_search_paths}
    )
  mark_as_advanced(Vulkan_glslang-machineindependent_DEBUG_LIBRARY)

  find_library(Vulkan_glslang-genericcodegen_LIBRARY
    NAMES GenericCodeGen
    HINTS
      ${_Vulkan_hint_library_search_paths}
    )
  mark_as_advanced(Vulkan_glslang-genericcodegen_LIBRARY)

  find_library(Vulkan_glslang-genericcodegen_DEBUG_LIBRARY
    NAMES GenericCodeGend
    HINTS
      ${_Vulkan_hint_library_search_paths}
    )
  mark_as_advanced(Vulkan_glslang-genericcodegen_DEBUG_LIBRARY)

  find_library(Vulkan_glslang_LIBRARY
    NAMES glslang
    HINTS
      ${_Vulkan_hint_library_search_paths}
    )
  mark_as_advanced(Vulkan_glslang_LIBRARY)

  find_library(Vulkan_glslang_DEBUG_LIBRARY
    NAMES glslangd
    HINTS
      ${_Vulkan_hint_library_search_paths}
    )
  mark_as_advanced(Vulkan_glslang_DEBUG_LIBRARY)
endif()
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
if(SPIRV-Tools IN_LIST Vulkan_FIND_COMPONENTS)
  find_library(Vulkan_SPIRV-Tools_LIBRARY
    NAMES SPIRV-Tools
    HINTS
      ${_Vulkan_hint_library_search_paths})
  mark_as_advanced(Vulkan_SPIRV-Tools_LIBRARY)

  find_library(Vulkan_SPIRV-Tools_DEBUG_LIBRARY
    NAMES SPIRV-Toolsd
    HINTS
      ${_Vulkan_hint_library_search_paths})
  mark_as_advanced(Vulkan_SPIRV-Tools_DEBUG_LIBRARY)
endif()
if(MoltenVK IN_LIST Vulkan_FIND_COMPONENTS)
  find_library(Vulkan_MoltenVK_LIBRARY
    NAMES MoltenVK
    HINTS
      ${_Vulkan_hint_library_search_paths})
  mark_as_advanced(Vulkan_MoltenVK_LIBRARY)

  find_path(Vulkan_MoltenVK_INCLUDE_DIR
    NAMES MoltenVK/mvk_vulkan.h
    HINTS
      ${_Vulkan_hint_include_search_paths}
    )
  mark_as_advanced(Vulkan_MoltenVK_INCLUDE_DIR)
endif()
if(volk IN_LIST Vulkan_FIND_COMPONENTS)
  find_library(Vulkan_volk_LIBRARY
          NAMES volk
          HINTS
            ${_Vulkan_hint_library_search_paths})
  mark_as_advanced(Vulkan_Volk_LIBRARY)
endif()

if (dxc IN_LIST Vulkan_FIND_COMPONENTS)
  find_library(Vulkan_dxc_LIBRARY
          NAMES dxcompiler
          HINTS
            ${_Vulkan_hint_library_search_paths})
  mark_as_advanced(Vulkan_dxc_LIBRARY)

  find_program(Vulkan_dxc_EXECUTABLE
          NAMES dxc
          HINTS
            ${_Vulkan_hint_executable_search_paths})
  mark_as_advanced(Vulkan_dxc_EXECUTABLE)
endif()

if(Vulkan_GLSLC_EXECUTABLE)
  set(Vulkan_glslc_FOUND TRUE)
else()
  set(Vulkan_glslc_FOUND FALSE)
endif()

if(Vulkan_GLSLANG_VALIDATOR_EXECUTABLE)
  set(Vulkan_glslangValidator_FOUND TRUE)
else()
  set(Vulkan_glslangValidator_FOUND FALSE)
endif()

if (Vulkan_dxc_EXECUTABLE)
  set(Vulkan_dxc_exe_FOUND TRUE)
else()
  set(Vulkan_dxc_exe_FOUND FALSE)
endif()

function(_Vulkan_set_library_component_found component)
  cmake_parse_arguments(PARSE_ARGV 1 _ARG
    "NO_WARNING"
    ""
    "DEPENDENT_COMPONENTS")

  set(all_dependent_component_found TRUE)
  foreach(dependent_component IN LISTS _ARG_DEPENDENT_COMPONENTS)
    if(NOT Vulkan_${dependent_component}_FOUND)
      set(all_dependent_component_found FALSE)
      break()
    endif()
  endforeach()

  if(all_dependent_component_found AND (Vulkan_${component}_LIBRARY OR Vulkan_${component}_DEBUG_LIBRARY))
    set(Vulkan_${component}_FOUND TRUE PARENT_SCOPE)

    # For Windows Vulkan SDK, third party tools binaries are provided with different MSVC ABI:
    #   - Release binaries uses a runtime library
    #   - Debug binaries uses a debug runtime library
    # This lead to incompatibilities in linking for some configuration types due to CMake-default or project-configured selected MSVC ABI.
    if(WIN32 AND NOT _ARG_NO_WARNING)
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

_Vulkan_set_library_component_found(glslang-spirv NO_WARNING)
_Vulkan_set_library_component_found(glslang-oglcompiler NO_WARNING)
_Vulkan_set_library_component_found(glslang-osdependent NO_WARNING)
_Vulkan_set_library_component_found(glslang-machineindependent NO_WARNING)
_Vulkan_set_library_component_found(glslang-genericcodegen NO_WARNING)
_Vulkan_set_library_component_found(glslang DEPENDENT_COMPONENTS glslang-spirv)
_Vulkan_set_library_component_found(shaderc_combined)
_Vulkan_set_library_component_found(SPIRV-Tools)
_Vulkan_set_library_component_found(volk)
_Vulkan_set_library_component_found(dxc)

if(Vulkan_MoltenVK_INCLUDE_DIR AND Vulkan_MoltenVK_LIBRARY)
  set(Vulkan_MoltenVK_FOUND TRUE)
else()
  set(Vulkan_MoltenVK_FOUND FALSE)
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

if(Vulkan_MoltenVK_FOUND)
  set(Vulkan_MoltenVK_VERSION "")
  if(Vulkan_MoltenVK_INCLUDE_DIR)
    set(VK_MVK_MOLTENVK_H ${Vulkan_MoltenVK_INCLUDE_DIR}/MoltenVK/vk_mvk_moltenvk.h)
    if(EXISTS ${VK_MVK_MOLTENVK_H})
      file(STRINGS  ${VK_MVK_MOLTENVK_H} _Vulkan_MoltenVK_VERSION_MAJOR REGEX "^#define MVK_VERSION_MAJOR ")
      string(REGEX MATCHALL "[0-9]+" _Vulkan_MoltenVK_VERSION_MAJOR "${_Vulkan_MoltenVK_VERSION_MAJOR}")
      file(STRINGS  ${VK_MVK_MOLTENVK_H} _Vulkan_MoltenVK_VERSION_MINOR REGEX "^#define MVK_VERSION_MINOR ")
      string(REGEX MATCHALL "[0-9]+" _Vulkan_MoltenVK_VERSION_MINOR "${_Vulkan_MoltenVK_VERSION_MINOR}")
      file(STRINGS  ${VK_MVK_MOLTENVK_H} _Vulkan_MoltenVK_VERSION_PATCH REGEX "^#define MVK_VERSION_PATCH ")
      string(REGEX MATCHALL "[0-9]+" _Vulkan_MoltenVK_VERSION_PATCH "${_Vulkan_MoltenVK_VERSION_PATCH}")
      set(Vulkan_MoltenVK_VERSION "${_Vulkan_MoltenVK_VERSION_MAJOR}.${_Vulkan_MoltenVK_VERSION_MINOR}.${_Vulkan_MoltenVK_VERSION_PATCH}")
      unset(_Vulkan_MoltenVK_VERSION_MAJOR)
      unset(_Vulkan_MoltenVK_VERSION_MINOR)
      unset(_Vulkan_MoltenVK_VERSION_PATCH)
    endif()
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
  if((Vulkan_glslang-spirv_LIBRARY OR Vulkan_glslang-spirv_DEBUG_LIBRARY) AND NOT TARGET Vulkan::glslang-spirv)
    add_library(Vulkan::glslang-spirv STATIC IMPORTED)
    set_property(TARGET Vulkan::glslang-spirv
      PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES "${Vulkan_INCLUDE_DIRS}")
    if(Vulkan_glslang-spirv_LIBRARY)
      set_property(TARGET Vulkan::glslang-spirv APPEND
        PROPERTY
          IMPORTED_CONFIGURATIONS Release)
      set_property(TARGET Vulkan::glslang-spirv
        PROPERTY
          IMPORTED_LOCATION_RELEASE "${Vulkan_glslang-spirv_LIBRARY}")
    endif()
    if(Vulkan_glslang-spirv_DEBUG_LIBRARY)
      set_property(TARGET Vulkan::glslang-spirv APPEND
        PROPERTY
          IMPORTED_CONFIGURATIONS Debug)
      set_property(TARGET Vulkan::glslang-spirv
        PROPERTY
          IMPORTED_LOCATION_DEBUG "${Vulkan_glslang-spirv_DEBUG_LIBRARY}")
    endif()
  endif()

  if((Vulkan_glslang-oglcompiler_LIBRARY OR Vulkan_glslang-oglcompiler_DEBUG_LIBRARY) AND NOT TARGET Vulkan::glslang-oglcompiler)
    add_library(Vulkan::glslang-oglcompiler STATIC IMPORTED)
    set_property(TARGET Vulkan::glslang-oglcompiler
      PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES "${Vulkan_INCLUDE_DIRS}")
    if(Vulkan_glslang-oglcompiler_LIBRARY)
      set_property(TARGET Vulkan::glslang-oglcompiler APPEND
        PROPERTY
          IMPORTED_CONFIGURATIONS Release)
      set_property(TARGET Vulkan::glslang-oglcompiler
        PROPERTY
          IMPORTED_LOCATION_RELEASE "${Vulkan_glslang-oglcompiler_LIBRARY}")
    endif()
    if(Vulkan_glslang-oglcompiler_DEBUG_LIBRARY)
      set_property(TARGET Vulkan::glslang-oglcompiler APPEND
        PROPERTY
          IMPORTED_CONFIGURATIONS Debug)
      set_property(TARGET Vulkan::glslang-oglcompiler
        PROPERTY
          IMPORTED_LOCATION_DEBUG "${Vulkan_glslang-oglcompiler_DEBUG_LIBRARY}")
    endif()
  endif()

  if((Vulkan_glslang-osdependent_LIBRARY OR Vulkan_glslang-osdependent_DEBUG_LIBRARY) AND NOT TARGET Vulkan::glslang-osdependent)
    add_library(Vulkan::glslang-osdependent STATIC IMPORTED)
    set_property(TARGET Vulkan::glslang-osdependent
      PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES "${Vulkan_INCLUDE_DIRS}")
    if(Vulkan_glslang-osdependent_LIBRARY)
      set_property(TARGET Vulkan::glslang-osdependent APPEND
        PROPERTY
          IMPORTED_CONFIGURATIONS Release)
      set_property(TARGET Vulkan::glslang-osdependent
        PROPERTY
          IMPORTED_LOCATION_RELEASE "${Vulkan_glslang-osdependent_LIBRARY}")
    endif()
    if(Vulkan_glslang-osdependent_DEBUG_LIBRARY)
      set_property(TARGET Vulkan::glslang-osdependent APPEND
        PROPERTY
          IMPORTED_CONFIGURATIONS Debug)
      set_property(TARGET Vulkan::glslang-osdependent
        PROPERTY
          IMPORTED_LOCATION_DEBUG "${Vulkan_glslang-osdependent_DEBUG_LIBRARY}")
    endif()
  endif()

  if((Vulkan_glslang-machineindependent_LIBRARY OR Vulkan_glslang-machineindependent_DEBUG_LIBRARY) AND NOT TARGET Vulkan::glslang-machineindependent)
    add_library(Vulkan::glslang-machineindependent STATIC IMPORTED)
    set_property(TARGET Vulkan::glslang-machineindependent
      PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES "${Vulkan_INCLUDE_DIRS}")
    if(Vulkan_glslang-machineindependent_LIBRARY)
      set_property(TARGET Vulkan::glslang-machineindependent APPEND
        PROPERTY
          IMPORTED_CONFIGURATIONS Release)
      set_property(TARGET Vulkan::glslang-machineindependent
        PROPERTY
          IMPORTED_LOCATION_RELEASE "${Vulkan_glslang-machineindependent_LIBRARY}")
    endif()
    if(Vulkan_glslang-machineindependent_DEBUG_LIBRARY)
      set_property(TARGET Vulkan::glslang-machineindependent APPEND
        PROPERTY
          IMPORTED_CONFIGURATIONS Debug)
      set_property(TARGET Vulkan::glslang-machineindependent
        PROPERTY
          IMPORTED_LOCATION_DEBUG "${Vulkan_glslang-machineindependent_DEBUG_LIBRARY}")
    endif()
  endif()

  if((Vulkan_glslang-genericcodegen_LIBRARY OR Vulkan_glslang-genericcodegen_DEBUG_LIBRARY) AND NOT TARGET Vulkan::glslang-genericcodegen)
    add_library(Vulkan::glslang-genericcodegen STATIC IMPORTED)
    set_property(TARGET Vulkan::glslang-genericcodegen
      PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES "${Vulkan_INCLUDE_DIRS}")
    if(Vulkan_glslang-genericcodegen_LIBRARY)
      set_property(TARGET Vulkan::glslang-genericcodegen APPEND
        PROPERTY
          IMPORTED_CONFIGURATIONS Release)
      set_property(TARGET Vulkan::glslang-genericcodegen
        PROPERTY
          IMPORTED_LOCATION_RELEASE "${Vulkan_glslang-genericcodegen_LIBRARY}")
    endif()
    if(Vulkan_glslang-genericcodegen_DEBUG_LIBRARY)
      set_property(TARGET Vulkan::glslang-genericcodegen APPEND
        PROPERTY
          IMPORTED_CONFIGURATIONS Debug)
      set_property(TARGET Vulkan::glslang-genericcodegen
        PROPERTY
          IMPORTED_LOCATION_DEBUG "${Vulkan_glslang-genericcodegen_DEBUG_LIBRARY}")
    endif()
  endif()

  if((Vulkan_glslang_LIBRARY OR Vulkan_glslang_DEBUG_LIBRARY)
      AND TARGET Vulkan::glslang-spirv
      AND NOT TARGET Vulkan::glslang)
    add_library(Vulkan::glslang STATIC IMPORTED)
    set_property(TARGET Vulkan::glslang
      PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES "${Vulkan_INCLUDE_DIRS}")
    if(Vulkan_glslang_LIBRARY)
      set_property(TARGET Vulkan::glslang APPEND
        PROPERTY
          IMPORTED_CONFIGURATIONS Release)
      set_property(TARGET Vulkan::glslang
        PROPERTY
          IMPORTED_LOCATION_RELEASE "${Vulkan_glslang_LIBRARY}")
    endif()
    if(Vulkan_glslang_DEBUG_LIBRARY)
      set_property(TARGET Vulkan::glslang APPEND
        PROPERTY
          IMPORTED_CONFIGURATIONS Debug)
      set_property(TARGET Vulkan::glslang
        PROPERTY
          IMPORTED_LOCATION_DEBUG "${Vulkan_glslang_DEBUG_LIBRARY}")
    endif()
    target_link_libraries(Vulkan::glslang
      INTERFACE
        Vulkan::glslang-spirv
        # OGLCompiler library has been fully removed since version 14.0.0
        # OSDependent, MachineIndependent, and GenericCodeGen may also be removed in the future.
        # See https://github.com/KhronosGroup/glslang/issues/3462
        $<TARGET_NAME_IF_EXISTS:Vulkan::glslang-oglcompiler>
        $<TARGET_NAME_IF_EXISTS:Vulkan::glslang-osdependent>
        $<TARGET_NAME_IF_EXISTS:Vulkan::glslang-machineindependent>
        $<TARGET_NAME_IF_EXISTS:Vulkan::glslang-genericcodegen>
    )
  endif()

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

  if((Vulkan_SPIRV-Tools_LIBRARY OR Vulkan_SPIRV-Tools_DEBUG_LIBRARY) AND NOT TARGET Vulkan::SPIRV-Tools)
    add_library(Vulkan::SPIRV-Tools STATIC IMPORTED)
    set_property(TARGET Vulkan::SPIRV-Tools
      PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES "${Vulkan_INCLUDE_DIRS}")
    if(Vulkan_SPIRV-Tools_LIBRARY)
      set_property(TARGET Vulkan::SPIRV-Tools APPEND
        PROPERTY
          IMPORTED_CONFIGURATIONS Release)
      set_property(TARGET Vulkan::SPIRV-Tools
        PROPERTY
          IMPORTED_LOCATION_RELEASE "${Vulkan_SPIRV-Tools_LIBRARY}")
    endif()
    if(Vulkan_SPIRV-Tools_DEBUG_LIBRARY)
      set_property(TARGET Vulkan::SPIRV-Tools APPEND
        PROPERTY
          IMPORTED_CONFIGURATIONS Debug)
      set_property(TARGET Vulkan::SPIRV-Tools
        PROPERTY
          IMPORTED_LOCATION_DEBUG "${Vulkan_SPIRV-Tools_DEBUG_LIBRARY}")
    endif()
  endif()

  if(Vulkan_volk_LIBRARY AND NOT TARGET Vulkan::volk)
    add_library(Vulkan::volk STATIC IMPORTED)
    set_property(TARGET Vulkan::volk
            PROPERTY
              INTERFACE_INCLUDE_DIRECTORIES "${Vulkan_INCLUDE_DIRS}")
    set_property(TARGET Vulkan::volk APPEND
            PROPERTY
              IMPORTED_CONFIGURATIONS Release)
    set_property(TARGET Vulkan::volk APPEND
            PROPERTY
              IMPORTED_LOCATION_RELEASE "${Vulkan_volk_LIBRARY}")

    if (NOT WIN32)
      set_property(TARGET Vulkan::volk APPEND
              PROPERTY
                IMPORTED_LINK_INTERFACE_LIBRARIES dl)
    endif()
  endif()

  if (Vulkan_dxc_LIBRARY AND NOT TARGET Vulkan::dxc_lib)
    add_library(Vulkan::dxc_lib STATIC IMPORTED)
    set_property(TARGET Vulkan::dxc_lib
      PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES "${Vulkan_INCLUDE_DIRS}")
    set_property(TARGET Vulkan::dxc_lib APPEND
      PROPERTY
        IMPORTED_CONFIGURATIONS Release)
    set_property(TARGET Vulkan::dxc_lib APPEND
      PROPERTY
        IMPORTED_LOCATION_RELEASE "${Vulkan_dxc_LIBRARY}")
  endif()

  if(Vulkan_dxc_EXECUTABLE AND NOT TARGET Vulkan::dxc_exe)
    add_executable(Vulkan::dxc_exe IMPORTED)
    set_property(TARGET Vulkan::dxc_exe PROPERTY IMPORTED_LOCATION "${Vulkan_dxc_EXECUTABLE}")
  endif()

endif()

if(Vulkan_MoltenVK_FOUND)
  if(Vulkan_MoltenVK_LIBRARY AND NOT TARGET Vulkan::MoltenVK)
    add_library(Vulkan::MoltenVK SHARED IMPORTED)
    set_target_properties(Vulkan::MoltenVK
      PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${Vulkan_MoltenVK_INCLUDE_DIR}"
        IMPORTED_LOCATION "${Vulkan_MoltenVK_LIBRARY}"
    )
  endif()
endif()

unset(_Vulkan_library_name)
unset(_Vulkan_hint_include_search_paths)
unset(_Vulkan_hint_executable_search_paths)
unset(_Vulkan_hint_library_search_paths)

cmake_policy(POP)
