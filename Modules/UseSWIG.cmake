#.rst:
# UseSWIG
# -------
#
# SWIG module for CMake
#
# Defines the following macros:
#
# ::
#
#    SWIG_ADD_MODULE(name language [ files ])
#      - Define swig module with given name and specified language
#    SWIG_LINK_LIBRARIES(name [ libraries ])
#      - Link libraries to swig module
#
# All other macros are for internal use only.  To get the actual name of
# the swig module, use: ${SWIG_MODULE_${name}_REAL_NAME}.  Set Source
# files properties such as CPLUSPLUS and SWIG_FLAGS to specify special
# behavior of SWIG.  Also global CMAKE_SWIG_FLAGS can be used to add
# special flags to all swig calls.  Another special variable is
# CMAKE_SWIG_OUTDIR, it allows one to specify where to write all the
# swig generated module (swig -outdir option) The name-specific variable
# SWIG_MODULE_<name>_EXTRA_DEPS may be used to specify extra
# dependencies for the generated modules.  If the source file generated
# by swig need some special flag you can use
# set_source_files_properties( ${swig_generated_file_fullname}
#
# ::
#
#         PROPERTIES COMPILE_FLAGS "-bla")


#=============================================================================
# Copyright 2004-2009 Kitware, Inc.
# Copyright 2009 Mathieu Malaterre <mathieu.malaterre@gmail.com>
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

set(SWIG_CXX_EXTENSION "cxx")
set(SWIG_EXTRA_LIBRARIES "")

set(SWIG_PYTHON_EXTRA_FILE_EXTENSION "py")

#
# For given swig module initialize variables associated with it
#
macro(SWIG_MODULE_INITIALIZE name language)
  string(TOUPPER "${language}" swig_uppercase_language)
  string(TOLOWER "${language}" swig_lowercase_language)
  set(SWIG_MODULE_${name}_LANGUAGE "${swig_uppercase_language}")
  set(SWIG_MODULE_${name}_SWIG_LANGUAGE_FLAG "${swig_lowercase_language}")

  set(SWIG_MODULE_${name}_REAL_NAME "${name}")
  if("${SWIG_MODULE_${name}_LANGUAGE}" STREQUAL "UNKNOWN")
    message(FATAL_ERROR "SWIG Error: Language \"${language}\" not found")
  elseif("${SWIG_MODULE_${name}_LANGUAGE}" STREQUAL "PYTHON")
    # when swig is used without the -interface it will produce in the module.py
    # a 'import _modulename' statement, which implies having a corresponding
    # _modulename.so (*NIX), _modulename.pyd (Win32).
    set(SWIG_MODULE_${name}_REAL_NAME "_${name}")
  elseif("${SWIG_MODULE_${name}_LANGUAGE}" STREQUAL "PERL")
    set(SWIG_MODULE_${name}_EXTRA_FLAGS "-shadow")
  endif()
endmacro()

#
# For a given language, input file, and output file, determine extra files that
# will be generated. This is internal swig macro.
#

macro(SWIG_GET_EXTRA_OUTPUT_FILES language outfiles generatedpath infile)
  set(${outfiles} "")
  get_source_file_property(SWIG_GET_EXTRA_OUTPUT_FILES_module_basename
    ${infile} SWIG_MODULE_NAME)
  if(SWIG_GET_EXTRA_OUTPUT_FILES_module_basename STREQUAL "NOTFOUND")
    get_filename_component(SWIG_GET_EXTRA_OUTPUT_FILES_module_basename "${infile}" NAME_WE)
  endif()
  foreach(it ${SWIG_${language}_EXTRA_FILE_EXTENSION})
    set(${outfiles} ${${outfiles}}
      "${generatedpath}/${SWIG_GET_EXTRA_OUTPUT_FILES_module_basename}.${it}")
  endforeach()
endmacro()

#
# Take swig (*.i) file and add proper custom commands for it
#
macro(SWIG_ADD_SOURCE_TO_MODULE name outfiles infile)
  set(swig_full_infile ${infile})
  get_filename_component(swig_source_file_path "${infile}" PATH)
  get_filename_component(swig_source_file_name_we "${infile}" NAME_WE)
  get_source_file_property(swig_source_file_generated ${infile} GENERATED)
  get_source_file_property(swig_source_file_cplusplus ${infile} CPLUSPLUS)
  get_source_file_property(swig_source_file_flags ${infile} SWIG_FLAGS)
  if("${swig_source_file_flags}" STREQUAL "NOTFOUND")
    set(swig_source_file_flags "")
  endif()
  set(swig_source_file_fullname "${infile}")
  if(${swig_source_file_path} MATCHES "^${CMAKE_CURRENT_SOURCE_DIR}")
    string(REGEX REPLACE
      "^${CMAKE_CURRENT_SOURCE_DIR}" ""
      swig_source_file_relative_path
      "${swig_source_file_path}")
  else()
    if(${swig_source_file_path} MATCHES "^${CMAKE_CURRENT_BINARY_DIR}")
      string(REGEX REPLACE
        "^${CMAKE_CURRENT_BINARY_DIR}" ""
        swig_source_file_relative_path
        "${swig_source_file_path}")
      set(swig_source_file_generated 1)
    else()
      set(swig_source_file_relative_path "${swig_source_file_path}")
      if(swig_source_file_generated)
        set(swig_source_file_fullname "${CMAKE_CURRENT_BINARY_DIR}/${infile}")
      else()
        set(swig_source_file_fullname "${CMAKE_CURRENT_SOURCE_DIR}/${infile}")
      endif()
    endif()
  endif()

  set(swig_generated_file_fullname
    "${CMAKE_CURRENT_BINARY_DIR}")
  if(swig_source_file_relative_path)
    set(swig_generated_file_fullname
      "${swig_generated_file_fullname}/${swig_source_file_relative_path}")
  endif()
  # If CMAKE_SWIG_OUTDIR was specified then pass it to -outdir
  if(CMAKE_SWIG_OUTDIR)
    set(swig_outdir ${CMAKE_SWIG_OUTDIR})
  else()
    set(swig_outdir ${CMAKE_CURRENT_BINARY_DIR})
  endif()
  SWIG_GET_EXTRA_OUTPUT_FILES(${SWIG_MODULE_${name}_LANGUAGE}
    swig_extra_generated_files
    "${swig_outdir}"
    "${infile}")
  set(swig_generated_file_fullname
    "${swig_generated_file_fullname}/${swig_source_file_name_we}")
  # add the language into the name of the file (i.e. TCL_wrap)
  # this allows for the same .i file to be wrapped into different languages
  set(swig_generated_file_fullname
    "${swig_generated_file_fullname}${SWIG_MODULE_${name}_LANGUAGE}_wrap")

  if(swig_source_file_cplusplus)
    set(swig_generated_file_fullname
      "${swig_generated_file_fullname}.${SWIG_CXX_EXTENSION}")
  else()
    set(swig_generated_file_fullname
      "${swig_generated_file_fullname}.c")
  endif()

  #message("Full path to source file: ${swig_source_file_fullname}")
  #message("Full path to the output file: ${swig_generated_file_fullname}")
  get_directory_property(cmake_include_directories INCLUDE_DIRECTORIES)
  set(swig_include_dirs)
  foreach(it ${cmake_include_directories})
    set(swig_include_dirs ${swig_include_dirs} "-I${it}")
  endforeach()

  set(swig_special_flags)
  # default is c, so add c++ flag if it is c++
  if(swig_source_file_cplusplus)
    set(swig_special_flags ${swig_special_flags} "-c++")
  endif()
  set(swig_extra_flags)
  if(SWIG_MODULE_${name}_EXTRA_FLAGS)
    set(swig_extra_flags ${swig_extra_flags} ${SWIG_MODULE_${name}_EXTRA_FLAGS})
  endif()
  add_custom_command(
    OUTPUT "${swig_generated_file_fullname}" ${swig_extra_generated_files}
    # Let's create the ${swig_outdir} at execution time, in case dir contains $(OutDir)
    COMMAND ${CMAKE_COMMAND} -E make_directory ${swig_outdir}
    COMMAND "${SWIG_EXECUTABLE}"
    ARGS "-${SWIG_MODULE_${name}_SWIG_LANGUAGE_FLAG}"
    ${swig_source_file_flags}
    ${CMAKE_SWIG_FLAGS}
    -outdir ${swig_outdir}
    ${swig_special_flags}
    ${swig_extra_flags}
    ${swig_include_dirs}
    -o "${swig_generated_file_fullname}"
    "${swig_source_file_fullname}"
    MAIN_DEPENDENCY "${swig_source_file_fullname}"
    DEPENDS ${SWIG_MODULE_${name}_EXTRA_DEPS}
    COMMENT "Swig source")
  set_source_files_properties("${swig_generated_file_fullname}" ${swig_extra_generated_files}
    PROPERTIES GENERATED 1)
  set(${outfiles} "${swig_generated_file_fullname}" ${swig_extra_generated_files})
endmacro()

#
# Create Swig module
#
macro(SWIG_ADD_MODULE name language)
  SWIG_MODULE_INITIALIZE(${name} ${language})
  set(swig_dot_i_sources)
  set(swig_other_sources)
  foreach(it ${ARGN})
    if(${it} MATCHES ".*\\.i$")
      set(swig_dot_i_sources ${swig_dot_i_sources} "${it}")
    else()
      set(swig_other_sources ${swig_other_sources} "${it}")
    endif()
  endforeach()

  set(swig_generated_sources)
  foreach(it ${swig_dot_i_sources})
    SWIG_ADD_SOURCE_TO_MODULE(${name} swig_generated_source ${it})
    set(swig_generated_sources ${swig_generated_sources} "${swig_generated_source}")
  endforeach()
  get_directory_property(swig_extra_clean_files ADDITIONAL_MAKE_CLEAN_FILES)
  set_directory_properties(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "${swig_extra_clean_files};${swig_generated_sources}")
  add_library(${SWIG_MODULE_${name}_REAL_NAME}
    MODULE
    ${swig_generated_sources}
    ${swig_other_sources})
  string(TOLOWER "${language}" swig_lowercase_language)
  if ("${swig_lowercase_language}" STREQUAL "java")
    if (APPLE)
        # In java you want:
        #      System.loadLibrary("LIBRARY");
        # then JNI will look for a library whose name is platform dependent, namely
        #   MacOS  : libLIBRARY.jnilib
        #   Windows: LIBRARY.dll
        #   Linux  : libLIBRARY.so
        set_target_properties (${SWIG_MODULE_${name}_REAL_NAME} PROPERTIES SUFFIX ".jnilib")
      endif ()
  endif ()
  if ("${swig_lowercase_language}" STREQUAL "python")
    # this is only needed for the python case where a _modulename.so is generated
    set_target_properties(${SWIG_MODULE_${name}_REAL_NAME} PROPERTIES PREFIX "")
    # Python extension modules on Windows must have the extension ".pyd"
    # instead of ".dll" as of Python 2.5.  Older python versions do support
    # this suffix.
    # http://docs.python.org/whatsnew/ports.html#SECTION0001510000000000000000
    # <quote>
    # Windows: .dll is no longer supported as a filename extension for extension modules.
    # .pyd is now the only filename extension that will be searched for.
    # </quote>
    if(WIN32 AND NOT CYGWIN)
      set_target_properties(${SWIG_MODULE_${name}_REAL_NAME} PROPERTIES SUFFIX ".pyd")
    endif()
  endif ()
endmacro()

#
# Like TARGET_LINK_LIBRARIES but for swig modules
#
macro(SWIG_LINK_LIBRARIES name)
  if(SWIG_MODULE_${name}_REAL_NAME)
    target_link_libraries(${SWIG_MODULE_${name}_REAL_NAME} ${ARGN})
  else()
    message(SEND_ERROR "Cannot find Swig library \"${name}\".")
  endif()
endmacro()

