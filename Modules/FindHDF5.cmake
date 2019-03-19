# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindHDF5
--------

Find HDF5, a library for reading and writing self describing array data.


This module invokes the HDF5 wrapper compiler that should be installed
alongside HDF5.  Depending upon the HDF5 Configuration, the wrapper
compiler is called either h5cc or h5pcc.  If this succeeds, the module
will then call the compiler with the -show argument to see what flags
are used when compiling an HDF5 client application.

The module will optionally accept the COMPONENTS argument.  If no
COMPONENTS are specified, then the find module will default to finding
only the HDF5 C library.  If one or more COMPONENTS are specified, the
module will attempt to find the language bindings for the specified
components.  The only valid components are C, CXX, Fortran, HL, and
Fortran_HL.  If the COMPONENTS argument is not given, the module will
attempt to find only the C bindings.

This module will read the variable
HDF5_USE_STATIC_LIBRARIES to determine whether or not to prefer a
static link to a dynamic link for HDF5 and all of it's dependencies.
To use this feature, make sure that the HDF5_USE_STATIC_LIBRARIES
variable is set before the call to find_package.

To provide the module with a hint about where to find your HDF5
installation, you can set the environment variable HDF5_ROOT.  The
Find module will then look in this path when searching for HDF5
executables, paths, and libraries.

Both the serial and parallel HDF5 wrappers are considered and the first
directory to contain either one will be used.  In the event that both appear
in the same directory the serial version is preferentially selected. This
behavior can be reversed by setting the variable HDF5_PREFER_PARALLEL to
true.

In addition to finding the includes and libraries required to compile
an HDF5 client application, this module also makes an effort to find
tools that come with the HDF5 distribution that may be useful for
regression testing.

Result Variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

``HDF5_FOUND``
  HDF5 was found on the system
``HDF5_VERSION``
  HDF5 library version
``HDF5_INCLUDE_DIRS``
  Location of the HDF5 header files
``HDF5_DEFINITIONS``
  Required compiler definitions for HDF5
``HDF5_LIBRARIES``
  Required libraries for all requested bindings
``HDF5_HL_LIBRARIES``
  Required libraries for the HDF5 high level API for all bindings,
  if the ``HL`` component is enabled

Available components are: ``C`` ``CXX`` ``Fortran`` and ``HL``.
For each enabled language binding, a corresponding ``HDF5_${LANG}_LIBRARIES``
variable, and potentially ``HDF5_${LANG}_DEFINITIONS``, will be defined.
If the ``HL`` component is enabled, then an ``HDF5_${LANG}_HL_LIBRARIES`` will
also be defined.  With all components enabled, the following variables will be defined:

``HDF5_C_DEFINITIONS``
  Required compiler definitions for HDF5 C bindings
``HDF5_CXX_DEFINITIONS``
  Required compiler definitions for HDF5 C++ bindings
``HDF5_Fortran_DEFINITIONS``
  Required compiler definitions for HDF5 Fortran bindings
``HDF5_C_INCLUDE_DIRS``
  Required include directories for HDF5 C bindings
``HDF5_CXX_INCLUDE_DIRS``
  Required include directories for HDF5 C++ bindings
``HDF5_Fortran_INCLUDE_DIRS``
  Required include directories for HDF5 Fortran bindings
``HDF5_C_LIBRARIES``
  Required libraries for the HDF5 C bindings
``HDF5_CXX_LIBRARIES``
  Required libraries for the HDF5 C++ bindings
``HDF5_Fortran_LIBRARIES``
  Required libraries for the HDF5 Fortran bindings
``HDF5_C_HL_LIBRARIES``
  Required libraries for the high level C bindings
``HDF5_CXX_HL_LIBRARIES``
  Required libraries for the high level C++ bindings
``HDF5_Fortran_HL_LIBRARIES``
  Required libraries for the high level Fortran bindings.

``HDF5_IS_PARALLEL``
  HDF5 library has parallel IO support
``HDF5_C_COMPILER_EXECUTABLE``
  path to the HDF5 C wrapper compiler
``HDF5_CXX_COMPILER_EXECUTABLE``
  path to the HDF5 C++ wrapper compiler
``HDF5_Fortran_COMPILER_EXECUTABLE``
  path to the HDF5 Fortran wrapper compiler
``HDF5_C_COMPILER_EXECUTABLE_NO_INTERROGATE``
  path to the primary C compiler which is also the HDF5 wrapper
``HDF5_CXX_COMPILER_EXECUTABLE_NO_INTERROGATE``
  path to the primary C++ compiler which is also the HDF5 wrapper
``HDF5_Fortran_COMPILER_EXECUTABLE_NO_INTERROGATE``
  path to the primary Fortran compiler which is also the HDF5 wrapper
``HDF5_DIFF_EXECUTABLE``
  path to the HDF5 dataset comparison tool

Hints
^^^^^

The following variable can be set to guide the search for HDF5 libraries and includes:

``HDF5_ROOT``
  Specify the path to the HDF5 installation to use.

``HDF5_FIND_DEBUG``
  Set ``true`` to get extra debugging output.

``HDF5_NO_FIND_PACKAGE_CONFIG_FILE``
  Set ``true`` to skip trying to find ``hdf5-config.cmake``.
#]=======================================================================]

# This module is maintained by Will Dicharry <wdicharry@stellarscience.com>.

include(${CMAKE_CURRENT_LIST_DIR}/SelectLibraryConfigurations.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)

# List of the valid HDF5 components
set(HDF5_VALID_LANGUAGE_BINDINGS C CXX Fortran)

# Validate the list of find components.
if(NOT HDF5_FIND_COMPONENTS)
  set(HDF5_LANGUAGE_BINDINGS "C")
else()
  set(HDF5_LANGUAGE_BINDINGS)
  # add the extra specified components, ensuring that they are valid.
  set(FIND_HL OFF)
  foreach(component IN LISTS HDF5_FIND_COMPONENTS)
    list(FIND HDF5_VALID_LANGUAGE_BINDINGS ${component} component_location)
    if(NOT component_location EQUAL -1)
      list(APPEND HDF5_LANGUAGE_BINDINGS ${component})
    elseif(component STREQUAL "HL")
      set(FIND_HL ON)
    elseif(component STREQUAL "Fortran_HL") # only for compatibility
      list(APPEND HDF5_LANGUAGE_BINDINGS Fortran)
      set(FIND_HL ON)
      set(HDF5_FIND_REQUIRED_Fortran_HL False)
      set(HDF5_FIND_REQUIRED_Fortran True)
      set(HDF5_FIND_REQUIRED_HL True)
    else()
      message(FATAL_ERROR "${component} is not a valid HDF5 component.")
    endif()
  endforeach()
  if(NOT HDF5_LANGUAGE_BINDINGS)
    get_property(__langs GLOBAL PROPERTY ENABLED_LANGUAGES)
    foreach(__lang IN LISTS __langs)
      if(__lang MATCHES "^(C|CXX|Fortran)$")
        list(APPEND HDF5_LANGUAGE_BINDINGS ${__lang})
      endif()
    endforeach()
  endif()
  list(REMOVE_ITEM HDF5_FIND_COMPONENTS Fortran_HL) # replaced by Fortran and HL
  list(REMOVE_DUPLICATES HDF5_LANGUAGE_BINDINGS)
endif()

# Determine whether to search for serial or parallel executable first
if(HDF5_PREFER_PARALLEL)
  set(HDF5_C_COMPILER_NAMES h5pcc h5cc)
  set(HDF5_CXX_COMPILER_NAMES h5pc++ h5c++)
  set(HDF5_Fortran_COMPILER_NAMES h5pfc h5fc)
else()
  set(HDF5_C_COMPILER_NAMES h5cc h5pcc)
  set(HDF5_CXX_COMPILER_NAMES h5c++ h5pc++)
  set(HDF5_Fortran_COMPILER_NAMES h5fc h5pfc)
endif()

# We may have picked up some duplicates in various lists during the above
# process for the language bindings (both the C and C++ bindings depend on
# libz for example).  Remove the duplicates. It appears that the default
# CMake behavior is to remove duplicates from the end of a list. However,
# for link lines, this is incorrect since unresolved symbols are searched
# for down the link line. Therefore, we reverse the list, remove the
# duplicates, and then reverse it again to get the duplicates removed from
# the beginning.
macro(_HDF5_remove_duplicates_from_beginning _list_name)
  if(${_list_name})
    list(REVERSE ${_list_name})
    list(REMOVE_DUPLICATES ${_list_name})
    list(REVERSE ${_list_name})
  endif()
endmacro()


# Test first if the current compilers automatically wrap HDF5

function(_HDF5_test_regular_compiler_C success version is_parallel)
  set(scratch_directory
    ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/hdf5)
  if(NOT ${success} OR
     NOT EXISTS ${scratch_directory}/compiler_has_h5_c)
    set(test_file ${scratch_directory}/cmake_hdf5_test.c)
    file(WRITE ${test_file}
      "#include <hdf5.h>\n"
      "#include <hdf5_hl.h>\n"
      "const char* info_ver = \"INFO\" \":\" H5_VERSION;\n"
      "#ifdef H5_HAVE_PARALLEL\n"
      "const char* info_parallel = \"INFO\" \":\" \"PARALLEL\";\n"
      "#endif\n"
      "int main(int argc, char **argv) {\n"
      "  int require = 0;\n"
      "  require += info_ver[argc];\n"
      "#ifdef H5_HAVE_PARALLEL\n"
      "  require += info_parallel[argc];\n"
      "#endif\n"
      "  hid_t fid;\n"
      "  fid = H5Fcreate(\"foo.h5\",H5F_ACC_TRUNC,H5P_DEFAULT,H5P_DEFAULT);\n"
      "  return 0;\n"
      "}")
    try_compile(${success} ${scratch_directory} ${test_file}
      COPY_FILE ${scratch_directory}/compiler_has_h5_c
    )
  endif()
  if(${success})
    file(STRINGS ${scratch_directory}/compiler_has_h5_c INFO_STRINGS
      REGEX "^INFO:"
    )
    string(REGEX MATCH "^INFO:([0-9]+\\.[0-9]+\\.[0-9]+)(-patch([0-9]+))?"
      INFO_VER "${INFO_STRINGS}"
    )
    set(${version} ${CMAKE_MATCH_1})
    if(CMAKE_MATCH_3)
      set(${version} ${HDF5_C_VERSION}.${CMAKE_MATCH_3})
    endif()
    set(${version} ${${version}} PARENT_SCOPE)

    if(INFO_STRINGS MATCHES "INFO:PARALLEL")
      set(${is_parallel} TRUE PARENT_SCOPE)
    else()
      set(${is_parallel} FALSE PARENT_SCOPE)
    endif()
  endif()
endfunction()

function(_HDF5_test_regular_compiler_CXX success version is_parallel)
  set(scratch_directory ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/hdf5)
  if(NOT ${success} OR
     NOT EXISTS ${scratch_directory}/compiler_has_h5_cxx)
    set(test_file ${scratch_directory}/cmake_hdf5_test.cxx)
    file(WRITE ${test_file}
      "#include <H5Cpp.h>\n"
      "#ifndef H5_NO_NAMESPACE\n"
      "using namespace H5;\n"
      "#endif\n"
      "const char* info_ver = \"INFO\" \":\" H5_VERSION;\n"
      "#ifdef H5_HAVE_PARALLEL\n"
      "const char* info_parallel = \"INFO\" \":\" \"PARALLEL\";\n"
      "#endif\n"
      "int main(int argc, char **argv) {\n"
      "  int require = 0;\n"
      "  require += info_ver[argc];\n"
      "#ifdef H5_HAVE_PARALLEL\n"
      "  require += info_parallel[argc];\n"
      "#endif\n"
      "  H5File file(\"foo.h5\", H5F_ACC_TRUNC);\n"
      "  return 0;\n"
      "}")
    try_compile(${success} ${scratch_directory} ${test_file}
      COPY_FILE ${scratch_directory}/compiler_has_h5_cxx
    )
  endif()
  if(${success})
    file(STRINGS ${scratch_directory}/compiler_has_h5_cxx INFO_STRINGS
      REGEX "^INFO:"
    )
    string(REGEX MATCH "^INFO:([0-9]+\\.[0-9]+\\.[0-9]+)(-patch([0-9]+))?"
      INFO_VER "${INFO_STRINGS}"
    )
    set(${version} ${CMAKE_MATCH_1})
    if(CMAKE_MATCH_3)
      set(${version} ${HDF5_CXX_VERSION}.${CMAKE_MATCH_3})
    endif()
    set(${version} ${${version}} PARENT_SCOPE)

    if(INFO_STRINGS MATCHES "INFO:PARALLEL")
      set(${is_parallel} TRUE PARENT_SCOPE)
    else()
      set(${is_parallel} FALSE PARENT_SCOPE)
    endif()
  endif()
endfunction()

function(_HDF5_test_regular_compiler_Fortran success is_parallel)
  if(NOT ${success})
    set(scratch_directory
      ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/hdf5)
    set(test_file ${scratch_directory}/cmake_hdf5_test.f90)
    file(WRITE ${test_file}
      "program hdf5_hello\n"
      "  use hdf5\n"
      "  use h5lt\n"
      "  use h5ds\n"
      "  integer error\n"
      "  call h5open_f(error)\n"
      "  call h5close_f(error)\n"
      "end\n")
    try_compile(${success} ${scratch_directory} ${test_file})
    if(${success})
      execute_process(COMMAND ${CMAKE_Fortran_COMPILER} -showconfig
        OUTPUT_VARIABLE config_output
        ERROR_VARIABLE config_error
        RESULT_VARIABLE config_result
        )
      if(config_output MATCHES "Parallel HDF5: yes")
        set(${is_parallel} TRUE PARENT_SCOPE)
      else()
        set(${is_parallel} FALSE PARENT_SCOPE)
      endif()
    endif()
  endif()
endfunction()

# Invoke the HDF5 wrapper compiler.  The compiler return value is stored to the
# return_value argument, the text output is stored to the output variable.
macro( _HDF5_invoke_compiler language output return_value version is_parallel)
    set(${version})
    if(HDF5_USE_STATIC_LIBRARIES)
        set(lib_type_args -noshlib)
    else()
        set(lib_type_args -shlib)
    endif()
    set(scratch_dir ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/hdf5)
    if("${language}" STREQUAL "C")
        set(test_file ${scratch_dir}/cmake_hdf5_test.c)
    elseif("${language}" STREQUAL "CXX")
        set(test_file ${scratch_dir}/cmake_hdf5_test.cxx)
    elseif("${language}" STREQUAL "Fortran")
        set(test_file ${scratch_dir}/cmake_hdf5_test.f90)
    endif()
    execute_process(
      COMMAND ${HDF5_${language}_COMPILER_EXECUTABLE} -show ${lib_type_args} ${test_file}
      OUTPUT_VARIABLE ${output}
      ERROR_VARIABLE ${output}
      RESULT_VARIABLE ${return_value}
      )
    if(NOT ${${return_value}} EQUAL 0)
        message(STATUS
          "Unable to determine HDF5 ${language} flags from HDF5 wrapper.")
    endif()
    execute_process(
      COMMAND ${HDF5_${language}_COMPILER_EXECUTABLE} -showconfig
      OUTPUT_VARIABLE config_output
      ERROR_VARIABLE config_output
      RESULT_VARIABLE config_return
      )
    if(NOT ${return_value} EQUAL 0)
        message( STATUS
          "Unable to determine HDF5 ${language} version from HDF5 wrapper.")
    endif()
    string(REGEX MATCH "HDF5 Version: ([a-zA-Z0-9\\.\\-]*)" version_match "${config_output}")
    if(version_match)
        string(REPLACE "HDF5 Version: " "" ${version} "${version_match}")
        string(REPLACE "-patch" "." ${version} "${${version}}")
    endif()
    if(config_output MATCHES "Parallel HDF5: yes")
      set(${is_parallel} TRUE)
    else()
      set(${is_parallel} FALSE)
    endif()
endmacro()

# Parse a compile line for definitions, includes, library paths, and libraries.
macro( _HDF5_parse_compile_line
    compile_line_var
    include_paths
    definitions
    library_paths
    libraries
    libraries_hl)

  separate_arguments(_HDF5_COMPILE_ARGS NATIVE_COMMAND "${${compile_line_var}}")

  foreach(arg IN LISTS _HDF5_COMPILE_ARGS)
    if("${arg}" MATCHES "^-I(.*)$")
      # include directory
      list(APPEND ${include_paths} "${CMAKE_MATCH_1}")
    elseif("${arg}" MATCHES "^-D(.*)$")
      # compile definition
      list(APPEND ${definitions} "-D${CMAKE_MATCH_1}")
    elseif("${arg}" MATCHES "^-L(.*)$")
      # library search path
      list(APPEND ${library_paths} "${CMAKE_MATCH_1}")
    elseif("${arg}" MATCHES "^-l(hdf5.*hl.*)$")
      # library name (hl)
      list(APPEND ${libraries_hl} "${CMAKE_MATCH_1}")
    elseif("${arg}" MATCHES "^-l(.*)$")
      # library name
      list(APPEND ${libraries} "${CMAKE_MATCH_1}")
    elseif("${arg}" MATCHES "^(.:)?[/\\].*\\.(a|so|dylib|sl|lib)$")
      # library file
      if(NOT EXISTS "${arg}")
        continue()
      endif()
      get_filename_component(_HDF5_LPATH "${arg}" DIRECTORY)
      get_filename_component(_HDF5_LNAME "${arg}" NAME_WE)
      string(REGEX REPLACE "^lib" "" _HDF5_LNAME "${_HDF5_LNAME}")
      list(APPEND ${library_paths} "${_HDF5_LPATH}")
      if(_HDF5_LNAME MATCHES "hdf5.*hl")
        list(APPEND ${libraries_hl} "${_HDF5_LNAME}")
      else()
        list(APPEND ${libraries} "${_HDF5_LNAME}")
      endif()
    endif()
  endforeach()
endmacro()

# Select a preferred imported configuration from a target
function(_HDF5_select_imported_config target imported_conf)
    # We will first assign the value to a local variable _imported_conf, then assign
    # it to the function argument at the end.
    get_target_property(_imported_conf ${target} MAP_IMPORTED_CONFIG_${CMAKE_BUILD_TYPE})
    if (NOT _imported_conf)
        # Get available imported configurations by examining target properties
        get_target_property(_imported_conf ${target} IMPORTED_CONFIGURATIONS)
        if(HDF5_FIND_DEBUG)
            message(STATUS "Found imported configurations: ${_imported_conf}")
        endif()
        # Find the imported configuration that we prefer.
        # We do this by making list of configurations in order of preference,
        # starting with ${CMAKE_BUILD_TYPE} and ending with the first imported_conf
        set(_preferred_confs ${CMAKE_BUILD_TYPE})
        list(GET _imported_conf 0 _fallback_conf)
        list(APPEND _preferred_confs RELWITHDEBINFO RELEASE DEBUG ${_fallback_conf})
        if(HDF5_FIND_DEBUG)
            message(STATUS "Start search through imported configurations in the following order: ${_preferred_confs}")
        endif()
        # Now find the first of these that is present in imported_conf
        cmake_policy(PUSH)
        cmake_policy(SET CMP0057 NEW) # support IN_LISTS
        foreach (_conf IN LISTS _preferred_confs)
            if (${_conf} IN_LIST _imported_conf)
               set(_imported_conf ${_conf})
               break()
            endif()
        endforeach()
        cmake_policy(POP)
    endif()
    if(HDF5_FIND_DEBUG)
        message(STATUS "Selected imported configuration: ${_imported_conf}")
    endif()
    # assign value to function argument
    set(${imported_conf} ${_imported_conf} PARENT_SCOPE)
endfunction()


if(NOT HDF5_ROOT)
    set(HDF5_ROOT $ENV{HDF5_ROOT})
endif()
if(HDF5_ROOT)
    set(_HDF5_SEARCH_OPTS NO_DEFAULT_PATH)
else()
    set(_HDF5_SEARCH_OPTS)
endif()

# Try to find HDF5 using an installed hdf5-config.cmake
if(NOT HDF5_FOUND AND NOT HDF5_NO_FIND_PACKAGE_CONFIG_FILE)
    find_package(HDF5 QUIET NO_MODULE
      HINTS ${HDF5_ROOT}
      ${_HDF5_SEARCH_OPTS}
      )
    if( HDF5_FOUND)
        if(HDF5_FIND_DEBUG)
            message(STATUS "Found HDF5 at ${HDF5_DIR} via NO_MODULE. Now trying to extract locations etc.")
        endif()
        set(HDF5_IS_PARALLEL ${HDF5_ENABLE_PARALLEL})
        set(HDF5_INCLUDE_DIRS ${HDF5_INCLUDE_DIR})
        set(HDF5_LIBRARIES)
        if (NOT TARGET hdf5 AND NOT TARGET hdf5-static AND NOT TARGET hdf5-shared)
            # Some HDF5 versions (e.g. 1.8.18) used hdf5::hdf5 etc
            set(_target_prefix "hdf5::")
        endif()
        set(HDF5_C_TARGET ${_target_prefix}hdf5)
        set(HDF5_C_HL_TARGET ${_target_prefix}hdf5_hl)
        set(HDF5_CXX_TARGET ${_target_prefix}hdf5_cpp)
        set(HDF5_CXX_HL_TARGET ${_target_prefix}hdf5_hl_cpp)
        set(HDF5_Fortran_TARGET ${_target_prefix}hdf5_fortran)
        set(HDF5_Fortran_HL_TARGET ${_target_prefix}hdf5_hl_fortran)
        set(HDF5_DEFINITIONS "")
        if(HDF5_USE_STATIC_LIBRARIES)
            set(_suffix "-static")
        else()
            set(_suffix "-shared")
        endif()
        foreach(_lang ${HDF5_LANGUAGE_BINDINGS})

            #Older versions of hdf5 don't have a static/shared suffix so
            #if we detect that occurrence clear the suffix
            if(_suffix AND NOT TARGET ${HDF5_${_lang}_TARGET}${_suffix})
              if(NOT TARGET ${HDF5_${_lang}_TARGET})
                #can't find this component with or without the suffix
                #so bail out, and let the following locate HDF5
                set(HDF5_FOUND FALSE)
                break()
              endif()
              set(_suffix "")
            endif()

            if(HDF5_FIND_DEBUG)
                message(STATUS "Trying to get properties of target ${HDF5_${_lang}_TARGET}${_suffix}")
            endif()
            # Find library for this target. Complicated as on Windows with a DLL, we need to search for the import-lib.
            _HDF5_select_imported_config(${HDF5_${_lang}_TARGET}${_suffix} _hdf5_imported_conf)
            get_target_property(_hdf5_lang_location ${HDF5_${_lang}_TARGET}${_suffix} IMPORTED_IMPLIB_${_hdf5_imported_conf} )
            if (NOT _hdf5_lang_location)
                # no import lib, just try LOCATION
                get_target_property(_hdf5_lang_location ${HDF5_${_lang}_TARGET}${_suffix} LOCATION_${_hdf5_imported_conf})
                if (NOT _hdf5_lang_location)
                    get_target_property(_hdf5_lang_location ${HDF5_${_lang}_TARGET}${_suffix} LOCATION)
                endif()
            endif()
            if( _hdf5_lang_location )
                set(HDF5_${_lang}_LIBRARY ${_hdf5_lang_location})
                list(APPEND HDF5_LIBRARIES ${HDF5_${_lang}_TARGET}${_suffix})
                set(HDF5_${_lang}_LIBRARIES ${HDF5_${_lang}_TARGET}${_suffix})
                set(HDF5_${_lang}_FOUND True)
            endif()
            if(FIND_HL)
                get_target_property(__lang_hl_location ${HDF5_${_lang}_HL_TARGET}${_suffix} IMPORTED_IMPLIB_${_hdf5_imported_conf} )
                if (NOT _hdf5_lang_hl_location)
                    get_target_property(_hdf5_lang_hl_location ${HDF5_${_lang}_HL_TARGET}${_suffix} LOCATION_${_hdf5_imported_conf})
                    if (NOT _hdf5_hl_lang_location)
                        get_target_property(_hdf5_hl_lang_location ${HDF5_${_lang}_HL_TARGET}${_suffix} LOCATION)
                    endif()
                endif()
                if( _hdf5_lang_hl_location )
                    set(HDF5_${_lang}_HL_LIBRARY ${_hdf5_lang_hl_location})
                    list(APPEND HDF5_HL_LIBRARIES ${HDF5_${_lang}_HL_TARGET}${_suffix})
                    set(HDF5_${_lang}_HL_LIBRARIES ${HDF5_${_lang}_HL_TARGET}${_suffix})
                    set(HDF5_HL_FOUND True)
                endif()
                unset(_hdf5_lang_hl_location)
            endif()
            unset(_hdf5_imported_conf)
            unset(_hdf5_lang_location)
        endforeach()
    endif()
endif()

if(NOT HDF5_FOUND)
  set(_HDF5_NEED_TO_SEARCH False)
  set(HDF5_COMPILER_NO_INTERROGATE True)
  # Only search for languages we've enabled
  foreach(__lang IN LISTS HDF5_LANGUAGE_BINDINGS)
    # First check to see if our regular compiler is one of wrappers
    if(__lang STREQUAL "C")
      _HDF5_test_regular_compiler_C(
        HDF5_${__lang}_COMPILER_NO_INTERROGATE
        HDF5_${__lang}_VERSION
        HDF5_${__lang}_IS_PARALLEL)
    elseif(__lang STREQUAL "CXX")
      _HDF5_test_regular_compiler_CXX(
        HDF5_${__lang}_COMPILER_NO_INTERROGATE
        HDF5_${__lang}_VERSION
        HDF5_${__lang}_IS_PARALLEL)
    elseif(__lang STREQUAL "Fortran")
      _HDF5_test_regular_compiler_Fortran(
        HDF5_${__lang}_COMPILER_NO_INTERROGATE
        HDF5_${__lang}_IS_PARALLEL)
    else()
      continue()
    endif()
    if(HDF5_${__lang}_COMPILER_NO_INTERROGATE)
      message(STATUS "HDF5: Using hdf5 compiler wrapper for all ${__lang} compiling")
      set(HDF5_${__lang}_FOUND True)
      set(HDF5_${__lang}_COMPILER_EXECUTABLE_NO_INTERROGATE
          "${CMAKE_${__lang}_COMPILER}"
          CACHE FILEPATH "HDF5 ${__lang} compiler wrapper")
      set(HDF5_${__lang}_DEFINITIONS)
      set(HDF5_${__lang}_INCLUDE_DIRS)
      set(HDF5_${__lang}_LIBRARIES)
      set(HDF5_${__lang}_HL_LIBRARIES)

      mark_as_advanced(HDF5_${__lang}_COMPILER_EXECUTABLE_NO_INTERROGATE)

      set(HDF5_${__lang}_FOUND True)
      set(HDF5_HL_FOUND True)
    else()
      set(HDF5_COMPILER_NO_INTERROGATE False)
      # If this language isn't using the wrapper, then try to seed the
      # search options with the wrapper
      find_program(HDF5_${__lang}_COMPILER_EXECUTABLE
        NAMES ${HDF5_${__lang}_COMPILER_NAMES} NAMES_PER_DIR
        HINTS ${HDF5_ROOT}
        PATH_SUFFIXES bin Bin
        DOC "HDF5 ${__lang} Wrapper compiler.  Used only to detect HDF5 compile flags."
        ${_HDF5_SEARCH_OPTS}
      )
      mark_as_advanced( HDF5_${__lang}_COMPILER_EXECUTABLE )
      unset(HDF5_${__lang}_COMPILER_NAMES)

      if(HDF5_${__lang}_COMPILER_EXECUTABLE)
        _HDF5_invoke_compiler(${__lang} HDF5_${__lang}_COMPILE_LINE
          HDF5_${__lang}_RETURN_VALUE HDF5_${__lang}_VERSION HDF5_${__lang}_IS_PARALLEL)
        if(HDF5_${__lang}_RETURN_VALUE EQUAL 0)
          message(STATUS "HDF5: Using hdf5 compiler wrapper to determine ${__lang} configuration")
          _HDF5_parse_compile_line( HDF5_${__lang}_COMPILE_LINE
            HDF5_${__lang}_INCLUDE_DIRS
            HDF5_${__lang}_DEFINITIONS
            HDF5_${__lang}_LIBRARY_DIRS
            HDF5_${__lang}_LIBRARY_NAMES
            HDF5_${__lang}_HL_LIBRARY_NAMES
          )
          set(HDF5_${__lang}_LIBRARIES)

          foreach(L IN LISTS HDF5_${__lang}_LIBRARY_NAMES)
            set(_HDF5_SEARCH_NAMES_LOCAL)
            if("x${L}" MATCHES "hdf5")
              # hdf5 library
              set(_HDF5_SEARCH_OPTS_LOCAL ${_HDF5_SEARCH_OPTS})
              if(HDF5_USE_STATIC_LIBRARIES)
                if(WIN32)
                  set(_HDF5_SEARCH_NAMES_LOCAL lib${L})
                else()
                  set(_HDF5_SEARCH_NAMES_LOCAL lib${L}.a)
                endif()
              endif()
            else()
              # external library
              set(_HDF5_SEARCH_OPTS_LOCAL)
            endif()
            find_library(HDF5_${__lang}_LIBRARY_${L}
              NAMES ${_HDF5_SEARCH_NAMES_LOCAL} ${L} NAMES_PER_DIR
              HINTS ${HDF5_${__lang}_LIBRARY_DIRS}
                    ${HDF5_ROOT}
              ${_HDF5_SEARCH_OPTS_LOCAL}
              )
            unset(_HDF5_SEARCH_OPTS_LOCAL)
            unset(_HDF5_SEARCH_NAMES_LOCAL)
            if(HDF5_${__lang}_LIBRARY_${L})
              list(APPEND HDF5_${__lang}_LIBRARIES ${HDF5_${__lang}_LIBRARY_${L}})
            else()
              list(APPEND HDF5_${__lang}_LIBRARIES ${L})
            endif()
          endforeach()
          if(FIND_HL)
            set(HDF5_${__lang}_HL_LIBRARIES)
            foreach(L IN LISTS HDF5_${__lang}_HL_LIBRARY_NAMES)
              set(_HDF5_SEARCH_NAMES_LOCAL)
              if("x${L}" MATCHES "hdf5")
                # hdf5 library
                set(_HDF5_SEARCH_OPTS_LOCAL ${_HDF5_SEARCH_OPTS})
                if(HDF5_USE_STATIC_LIBRARIES)
                  if(WIN32)
                    set(_HDF5_SEARCH_NAMES_LOCAL lib${L})
                  else()
                    set(_HDF5_SEARCH_NAMES_LOCAL lib${L}.a)
                  endif()
                endif()
              else()
                # external library
                set(_HDF5_SEARCH_OPTS_LOCAL)
              endif()
              find_library(HDF5_${__lang}_LIBRARY_${L}
                NAMES ${_HDF5_SEARCH_NAMES_LOCAL} ${L} NAMES_PER_DIR
                HINTS ${HDF5_${__lang}_LIBRARY_DIRS}
                      ${HDF5_ROOT}
                ${_HDF5_SEARCH_OPTS_LOCAL}
                )
              unset(_HDF5_SEARCH_OPTS_LOCAL)
              unset(_HDF5_SEARCH_NAMES_LOCAL)
              if(HDF5_${__lang}_LIBRARY_${L})
                list(APPEND HDF5_${__lang}_HL_LIBRARIES ${HDF5_${__lang}_LIBRARY_${L}})
              else()
                list(APPEND HDF5_${__lang}_HL_LIBRARIES ${L})
              endif()
            endforeach()
            set(HDF5_HL_FOUND True)
          endif()

          set(HDF5_${__lang}_FOUND True)
          _HDF5_remove_duplicates_from_beginning(HDF5_${__lang}_DEFINITIONS)
          _HDF5_remove_duplicates_from_beginning(HDF5_${__lang}_INCLUDE_DIRS)
          _HDF5_remove_duplicates_from_beginning(HDF5_${__lang}_LIBRARIES)
          _HDF5_remove_duplicates_from_beginning(HDF5_${__lang}_HL_LIBRARIES)
        else()
          set(_HDF5_NEED_TO_SEARCH True)
        endif()
      else()
        set(_HDF5_NEED_TO_SEARCH True)
      endif()
    endif()
    if(HDF5_${__lang}_VERSION)
      if(NOT HDF5_VERSION)
        set(HDF5_VERSION ${HDF5_${__lang}_VERSION})
      elseif(NOT HDF5_VERSION VERSION_EQUAL HDF5_${__lang}_VERSION)
        message(WARNING "HDF5 Version found for language ${__lang}, ${HDF5_${__lang}_VERSION} is different than previously found version ${HDF5_VERSION}")
      endif()
    endif()
    if(DEFINED HDF5_${__lang}_IS_PARALLEL)
      if(NOT DEFINED HDF5_IS_PARALLEL)
        set(HDF5_IS_PARALLEL ${HDF5_${__lang}_IS_PARALLEL})
      elseif(NOT HDF5_IS_PARALLEL AND HDF5_${__lang}_IS_PARALLEL)
        message(WARNING "HDF5 found for language ${__lang} is parallel but previously found language is not parallel.")
      elseif(HDF5_IS_PARALLEL AND NOT HDF5_${__lang}_IS_PARALLEL)
        message(WARNING "HDF5 found for language ${__lang} is not parallel but previously found language is parallel.")
      endif()
    endif()
  endforeach()
else()
  set(_HDF5_NEED_TO_SEARCH True)
endif()

if(NOT HDF5_FOUND AND HDF5_COMPILER_NO_INTERROGATE)
  # No arguments necessary, all languages can use the compiler wrappers
  set(HDF5_FOUND True)
  set(HDF5_METHOD "Included by compiler wrappers")
  set(HDF5_REQUIRED_VARS HDF5_METHOD)
elseif(NOT HDF5_FOUND AND NOT _HDF5_NEED_TO_SEARCH)
  # Compiler wrappers aren't being used by the build but were found and used
  # to determine necessary include and library flags
  set(HDF5_INCLUDE_DIRS)
  set(HDF5_LIBRARIES)
  set(HDF5_HL_LIBRARIES)
  foreach(__lang IN LISTS HDF5_LANGUAGE_BINDINGS)
    if(HDF5_${__lang}_FOUND)
      if(NOT HDF5_${__lang}_COMPILER_NO_INTERROGATE)
        list(APPEND HDF5_DEFINITIONS ${HDF5_${__lang}_DEFINITIONS})
        list(APPEND HDF5_INCLUDE_DIRS ${HDF5_${__lang}_INCLUDE_DIRS})
        list(APPEND HDF5_LIBRARIES ${HDF5_${__lang}_LIBRARIES})
        if(FIND_HL)
          list(APPEND HDF5_HL_LIBRARIES ${HDF5_${__lang}_HL_LIBRARIES})
        endif()
      endif()
    endif()
  endforeach()
  _HDF5_remove_duplicates_from_beginning(HDF5_DEFINITIONS)
  _HDF5_remove_duplicates_from_beginning(HDF5_INCLUDE_DIRS)
  _HDF5_remove_duplicates_from_beginning(HDF5_LIBRARIES)
  _HDF5_remove_duplicates_from_beginning(HDF5_HL_LIBRARIES)
  set(HDF5_FOUND True)
  set(HDF5_REQUIRED_VARS HDF5_LIBRARIES)
  if(FIND_HL)
    list(APPEND HDF5_REQUIRED_VARS HDF5_HL_LIBRARIES)
  endif()
endif()

find_program( HDF5_DIFF_EXECUTABLE
    NAMES h5diff
    HINTS ${HDF5_ROOT}
    PATH_SUFFIXES bin Bin
    ${_HDF5_SEARCH_OPTS}
    DOC "HDF5 file differencing tool." )
mark_as_advanced( HDF5_DIFF_EXECUTABLE )

if( NOT HDF5_FOUND )
    # seed the initial lists of libraries to find with items we know we need
    set(HDF5_C_LIBRARY_NAMES          hdf5)
    set(HDF5_C_HL_LIBRARY_NAMES       hdf5_hl)

    set(HDF5_CXX_LIBRARY_NAMES        hdf5_cpp    ${HDF5_C_LIBRARY_NAMES})
    set(HDF5_CXX_HL_LIBRARY_NAMES     hdf5_hl_cpp ${HDF5_C_HL_LIBRARY_NAMES} ${HDF5_CXX_LIBRARY_NAMES})

    set(HDF5_Fortran_LIBRARY_NAMES    hdf5_fortran   ${HDF5_C_LIBRARY_NAMES})
    set(HDF5_Fortran_HL_LIBRARY_NAMES hdf5hl_fortran ${HDF5_C_HL_LIBRARY_NAMES} ${HDF5_Fortran_LIBRARY_NAMES})

    foreach(__lang IN LISTS HDF5_LANGUAGE_BINDINGS)
        # find the HDF5 include directories
        if("${__lang}" STREQUAL "Fortran")
            set(HDF5_INCLUDE_FILENAME hdf5.mod)
        elseif("${__lang}" STREQUAL "CXX")
            set(HDF5_INCLUDE_FILENAME H5Cpp.h)
        else()
            set(HDF5_INCLUDE_FILENAME hdf5.h)
        endif()

        find_path(HDF5_${__lang}_INCLUDE_DIR ${HDF5_INCLUDE_FILENAME}
            HINTS ${HDF5_ROOT}
            PATHS $ENV{HOME}/.local/include
            PATH_SUFFIXES include Include
            ${_HDF5_SEARCH_OPTS}
        )
        mark_as_advanced(HDF5_${__lang}_INCLUDE_DIR)
        # set the _DIRS variable as this is what the user will normally use
        set(HDF5_${__lang}_INCLUDE_DIRS ${HDF5_${__lang}_INCLUDE_DIR})
        list(APPEND HDF5_INCLUDE_DIRS ${HDF5_${__lang}_INCLUDE_DIR})

        # find the HDF5 libraries
        foreach(LIB IN LISTS HDF5_${__lang}_LIBRARY_NAMES)
            if(HDF5_USE_STATIC_LIBRARIES)
                # According to bug 1643 on the CMake bug tracker, this is the
                # preferred method for searching for a static library.
                # See https://gitlab.kitware.com/cmake/cmake/issues/1643.  We search
                # first for the full static library name, but fall back to a
                # generic search on the name if the static search fails.
                set( THIS_LIBRARY_SEARCH_DEBUG
                    lib${LIB}d.a lib${LIB}_debug.a lib${LIB}d lib${LIB}_D lib${LIB}_debug
                    lib${LIB}d-static.a lib${LIB}_debug-static.a ${LIB}d-static ${LIB}_D-static ${LIB}_debug-static )
                set( THIS_LIBRARY_SEARCH_RELEASE lib${LIB}.a lib${LIB} lib${LIB}-static.a ${LIB}-static)
            else()
                set( THIS_LIBRARY_SEARCH_DEBUG ${LIB}d ${LIB}_D ${LIB}_debug ${LIB}d-shared ${LIB}_D-shared ${LIB}_debug-shared)
                set( THIS_LIBRARY_SEARCH_RELEASE ${LIB} ${LIB}-shared)
                if(WIN32)
                  list(APPEND HDF5_DEFINITIONS "-DH5_BUILT_AS_DYNAMIC_LIB")
                endif()
            endif()
            find_library(HDF5_${LIB}_LIBRARY_DEBUG
                NAMES ${THIS_LIBRARY_SEARCH_DEBUG}
                HINTS ${HDF5_ROOT} PATH_SUFFIXES lib Lib
                ${_HDF5_SEARCH_OPTS}
            )
            find_library( HDF5_${LIB}_LIBRARY_RELEASE
                NAMES ${THIS_LIBRARY_SEARCH_RELEASE}
                HINTS ${HDF5_ROOT} PATH_SUFFIXES lib Lib
                ${_HDF5_SEARCH_OPTS}
            )
            select_library_configurations( HDF5_${LIB} )
            list(APPEND HDF5_${__lang}_LIBRARIES ${HDF5_${LIB}_LIBRARY})
        endforeach()
        if(HDF5_${__lang}_LIBRARIES)
            set(HDF5_${__lang}_FOUND True)
        endif()

        # Append the libraries for this language binding to the list of all
        # required libraries.
        list(APPEND HDF5_LIBRARIES ${HDF5_${__lang}_LIBRARIES})

        if(FIND_HL)
            foreach(LIB IN LISTS HDF5_${__lang}_HL_LIBRARY_NAMES)
                if(HDF5_USE_STATIC_LIBRARIES)
                    # According to bug 1643 on the CMake bug tracker, this is the
                    # preferred method for searching for a static library.
                    # See https://gitlab.kitware.com/cmake/cmake/issues/1643.  We search
                    # first for the full static library name, but fall back to a
                    # generic search on the name if the static search fails.
                    set( THIS_LIBRARY_SEARCH_DEBUG
                        lib${LIB}d.a lib${LIB}_debug.a lib${LIB}d lib${LIB}_D lib${LIB}_debug
                        lib${LIB}d-static.a lib${LIB}_debug-static.a lib${LIB}d-static lib${LIB}_D-static lib${LIB}_debug-static )
                    set( THIS_LIBRARY_SEARCH_RELEASE lib${LIB}.a lib${LIB} lib${LIB}-static.a lib${LIB}-static)
                else()
                    set( THIS_LIBRARY_SEARCH_DEBUG ${LIB}d ${LIB}_D ${LIB}_debug ${LIB}d-shared ${LIB}_D-shared ${LIB}_debug-shared)
                    set( THIS_LIBRARY_SEARCH_RELEASE ${LIB} ${LIB}-shared)
                endif()
                find_library(HDF5_${LIB}_LIBRARY_DEBUG
                    NAMES ${THIS_LIBRARY_SEARCH_DEBUG}
                    HINTS ${HDF5_ROOT} PATH_SUFFIXES lib Lib
                    ${_HDF5_SEARCH_OPTS}
                )
                find_library( HDF5_${LIB}_LIBRARY_RELEASE
                    NAMES ${THIS_LIBRARY_SEARCH_RELEASE}
                    HINTS ${HDF5_ROOT} PATH_SUFFIXES lib Lib
                    ${_HDF5_SEARCH_OPTS}
                )
                select_library_configurations( HDF5_${LIB} )
                list(APPEND HDF5_${__lang}_HL_LIBRARIES ${HDF5_${LIB}_LIBRARY})
            endforeach()

            # Append the libraries for this language binding to the list of all
            # required libraries.
            list(APPEND HDF5_HL_LIBRARIES ${HDF5_${__lang}_HL_LIBRARIES})
        endif()
    endforeach()
    if(FIND_HL AND HDF5_HL_LIBRARIES)
        set(HDF5_HL_FOUND True)
    endif()

    _HDF5_remove_duplicates_from_beginning(HDF5_DEFINITIONS)
    _HDF5_remove_duplicates_from_beginning(HDF5_INCLUDE_DIRS)
    _HDF5_remove_duplicates_from_beginning(HDF5_LIBRARIES)
    _HDF5_remove_duplicates_from_beginning(HDF5_HL_LIBRARIES)

    # If the HDF5 include directory was found, open H5pubconf.h to determine if
    # HDF5 was compiled with parallel IO support
    set( HDF5_IS_PARALLEL FALSE )
    set( HDF5_VERSION "" )
    foreach( _dir IN LISTS HDF5_INCLUDE_DIRS )
      foreach(_hdr "${_dir}/H5pubconf.h" "${_dir}/H5pubconf-64.h" "${_dir}/H5pubconf-32.h")
        if( EXISTS "${_hdr}" )
            file( STRINGS "${_hdr}"
                HDF5_HAVE_PARALLEL_DEFINE
                REGEX "HAVE_PARALLEL 1" )
            if( HDF5_HAVE_PARALLEL_DEFINE )
                set( HDF5_IS_PARALLEL TRUE )
            endif()
            unset(HDF5_HAVE_PARALLEL_DEFINE)

            file( STRINGS "${_hdr}"
                HDF5_VERSION_DEFINE
                REGEX "^[ \t]*#[ \t]*define[ \t]+H5_VERSION[ \t]+" )
            if( "${HDF5_VERSION_DEFINE}" MATCHES
                "H5_VERSION[ \t]+\"([0-9]+\\.[0-9]+\\.[0-9]+)(-patch([0-9]+))?\"" )
                set( HDF5_VERSION "${CMAKE_MATCH_1}" )
                if( CMAKE_MATCH_3 )
                  set( HDF5_VERSION ${HDF5_VERSION}.${CMAKE_MATCH_3})
                endif()
            endif()
            unset(HDF5_VERSION_DEFINE)
        endif()
      endforeach()
    endforeach()
    set( HDF5_IS_PARALLEL ${HDF5_IS_PARALLEL} CACHE BOOL
        "HDF5 library compiled with parallel IO support" )
    mark_as_advanced( HDF5_IS_PARALLEL )

    set(HDF5_REQUIRED_VARS HDF5_LIBRARIES HDF5_INCLUDE_DIRS)
    if(FIND_HL)
        list(APPEND HDF5_REQUIRED_VARS HDF5_HL_LIBRARIES)
    endif()
endif()

# For backwards compatibility we set HDF5_INCLUDE_DIR to the value of
# HDF5_INCLUDE_DIRS
if( HDF5_INCLUDE_DIRS )
  set( HDF5_INCLUDE_DIR "${HDF5_INCLUDE_DIRS}" )
endif()

# If HDF5_REQUIRED_VARS is empty at this point, then it's likely that
# something external is trying to explicitly pass already found
# locations
if(NOT HDF5_REQUIRED_VARS)
    set(HDF5_REQUIRED_VARS HDF5_LIBRARIES HDF5_INCLUDE_DIRS)
endif()

find_package_handle_standard_args(HDF5
    REQUIRED_VARS ${HDF5_REQUIRED_VARS}
    VERSION_VAR   HDF5_VERSION
    HANDLE_COMPONENTS
)

unset(_HDF5_SEARCH_OPTS)

if( HDF5_FOUND AND NOT HDF5_DIR)
  # hide HDF5_DIR for the non-advanced user to avoid confusion with
  # HDF5_DIR-NOT_FOUND while HDF5 was found.
  mark_as_advanced(HDF5_DIR)
endif()

if (HDF5_FIND_DEBUG)
  message(STATUS "HDF5_DIR: ${HDF5_DIR}")
  message(STATUS "HDF5_DEFINITIONS: ${HDF5_DEFINITIONS}")
  message(STATUS "HDF5_INCLUDE_DIRS: ${HDF5_INCLUDE_DIRS}")
  message(STATUS "HDF5_LIBRARIES: ${HDF5_LIBRARIES}")
  message(STATUS "HDF5_HL_LIBRARIES: ${HDF5_HL_LIBRARIES}")
  foreach(__lang IN LISTS HDF5_LANGUAGE_BINDINGS)
    message(STATUS "HDF5_${__lang}_DEFINITIONS: ${HDF5_${__lang}_DEFINITIONS}")
    message(STATUS "HDF5_${__lang}_INCLUDE_DIR: ${HDF5_${__lang}_INCLUDE_DIR}")
    message(STATUS "HDF5_${__lang}_INCLUDE_DIRS: ${HDF5_${__lang}_INCLUDE_DIRS}")
    message(STATUS "HDF5_${__lang}_LIBRARY: ${HDF5_${__lang}_LIBRARY}")
    message(STATUS "HDF5_${__lang}_LIBRARIES: ${HDF5_${__lang}_LIBRARIES}")
    message(STATUS "HDF5_${__lang}_HL_LIBRARY: ${HDF5_${__lang}_HL_LIBRARY}")
    message(STATUS "HDF5_${__lang}_HL_LIBRARIES: ${HDF5_${__lang}_HL_LIBRARIES}")
  endforeach()
endif()
