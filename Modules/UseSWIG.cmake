# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
UseSWIG
-------

Defines the following command for use with SWIG:

.. command:: swig_add_library

  Define swig module with given name and specified language::

    swig_add_library(<name>
                     [TYPE <SHARED|MODULE|STATIC|USE_BUILD_SHARED_LIBS>]
                     LANGUAGE <language>
                     [NO_PROXY]
                     [OUTPUT_DIR <directory>]
                     [OUTFILE_DIR <directory>]
                     SOURCES <file>...
                    )

  Targets created with command ``swig_add_library`` have the same capabilities as targets
  created with command :command:`add_library`, so can be used with any command accepting a target
  especially command :command:`target_link_libraries`.

  The arguments are:

  ``TYPE``
    ``SHARED``, ``MODULE`` and ``STATIC`` have same semantic as command :command:`add_library`.
    if ``USE_BUILD_SHARED_LIBS`` is specified, library type will be ``STATIC`` or ``SHARED``
    based on whether the current value of the variable :variable:`BUILD_SHARED_LIBS` is ``ON``.
    If none is specified, ``MODULE`` will be used.

  ``LANGUAGE``
    Specify the target language.

  ``NO_PROXY``
    Prevent the generation of the wrapper layer (swig ``-noproxy`` option).

  ``OUTPUT_DIR``
    Specify where to write the language specific files (swig ``-outdir`` option).
    If not specified, variable ``CMAKE_SWIG_OUTDIR`` will be used. If none is specified,
    :variable:`CMAKE_CURRENT_BINARY_DIR` is used.

  ``OUTFILE_DIR``
    Specify an output directory name where the generated source file will be placed
    (swig -o option). If not specified, variable ``SWIG_OUTFILE_DIR`` will be used.
    If none is specified, option ``OUTPUT_DIR`` or variable ``CMAKE_SWIG_OUTDIR`` is used.

  ``SOURCES``
    List of sources for the library. Files with extension ``.i`` will be identified as sources
    for ``SWIG`` tool. Other files will be handled in the standard way.

Source files properties on module files **must** be set before the invocation
of the ``swig_add_library`` command to specify special behavior of SWIG and ensure
generated files will receive required settings.

``CPLUSPLUS``
  Call SWIG in c++ mode.  For example:

  .. code-block:: cmake

    set_property(SOURCE mymod.i PROPERTY CPLUSPLUS ON)
    swig_add_library(mymod LANGUAGE python SOURCES mymod.i)

``INCLUDE_DIRECTORIES``, ``COMPILE_DEFINITIONS`` and ``COMPILE_OPTIONS``
  Add custom flags to SWIG compiler and have same semantic as properties
  :prop_sf:`INCLUDE_DIRECTORIES`, :prop_sf:`COMPILE_DEFINITIONS` and :prop_sf:`COMPILE_OPTIONS`.

``GENERATED_INCLUDE_DIRECTORIES``, ``GENERATED_COMPILE_DEFINITIONS`` and ``GENERATED_COMPILE_OPTIONS``
  Add custom flags to the C/C++ generated source. They will fill, respectively,
  properties :prop_sf:`INCLUDE_DIRECTORIES`, :prop_sf:`COMPILE_DEFINITIONS` and
  :prop_sf:`COMPILE_OPTIONS` of generated C/C++ file.

``DEPENDS``
  Specify additional dependencies to the source file.

``SWIG_MODULE_NAME``
  Specify the actual import name of the module in the target language.
  This is required if it cannot be scanned automatically from source
  or different from the module file basename.  For example:

  .. code-block:: cmake

    set_property(SOURCE mymod.i PROPERTY SWIG_MODULE_NAME mymod_realname)

Target library properties can be set to apply same configuration to all SWIG input files.

``SWIG_INCLUDE_DIRECTORIES``, ``SWIG_COMPILE_DEFINITIONS`` and ``SWIG_COMPILE_OPTIONS``
  These properties will be applied to all SWIG input files and have same semantic as
  target properties :prop_tgt:`INCLUDE_DIRECTORIES`, :prop_tgt:`COMPILE_DEFINITIONS` and
  :prop_tgt:`COMPILE_OPTIONS`.

  .. code-block:: cmake

    swig_add_library(mymod LANGUAGE python SOURCES mymod.i)
    set_property(TARGET mymod PROPERTY SWIG_COMPILE_DEFINITIONS MY_DEF1 MY_DEF2)
    set_property(TARGET mymod PROPERTY SWIG_COMPILE_OPTIONS -bla -blb)

``SWIG_GENERATED_INCLUDE_DIRECTORIES``, ``SWIG_GENERATED_COMPILE_DEFINITIONS`` and ``SWIG_GENERATED_COMPILE_OPTIONS``
  These properties will populate, respectively, properties :prop_sf:`INCLUDE_DIRECTORIES`,
  :prop_sf:`COMPILE_DEFINITIONS` and :prop_sf:`COMPILE_FLAGS` of all generated C/C++ files.

``SWIG_DEPENDS``
  Add dependencies to all SWIG input files.

``SWIG_SUPPORT_FILES``
  This output property list of wrapper files generated during SWIG compilation.

  .. code-block:: cmake

    swig_add_library(mymod LANGUAGE python SOURCES mymod.i)
    get_property(support_files TARGET mymod PROPERTY SWIG_SUPPORT_FILES)

Some variables can be set to specify special behavior of SWIG:

``CMAKE_SWIG_FLAGS``
  Add flags to all swig calls.

``CMAKE_SWIG_OUTDIR``
  Specify where to write the language specific files (swig ``-outdir`` option).

``SWIG_OUTFILE_DIR``
  Specify an output directory name where the generated source file will be
  placed.  If not specified, ``CMAKE_SWIG_OUTDIR`` is used.

``SWIG_MODULE_<name>_EXTRA_DEPS``
  Specify extra dependencies for the generated module for ``<name>``.
#]=======================================================================]


cmake_policy (VERSION 3.11)

set(SWIG_CXX_EXTENSION "cxx")
set(SWIG_EXTRA_LIBRARIES "")

set(SWIG_PYTHON_EXTRA_FILE_EXTENSIONS ".py")
set(SWIG_JAVA_EXTRA_FILE_EXTENSIONS ".java" "JNI.java")

##
## PRIVATE functions
##
function (__SWIG_COMPUTE_TIMESTAMP name language infile workingdir __timestamp)
  get_filename_component(filename "${infile}" NAME_WE)
  set(${__timestamp}
    "${workingdir}/${filename}${language}.stamp" PARENT_SCOPE)
  # get_filename_component(filename "${infile}" ABSOLUTE)
  # string(UUID uuid NAMESPACE 9735D882-D2F8-4E1D-88C9-A0A4F1F6ECA4
  #   NAME ${name}-${language}-${filename} TYPE SHA1)
  # set(${__timestamp} "${workingdir}/${uuid}.stamp" PARENT_SCOPE)
endfunction()

#
# For given swig module initialize variables associated with it
#
macro(SWIG_MODULE_INITIALIZE name language)
  string(TOUPPER "${language}" SWIG_MODULE_${name}_LANGUAGE)
  string(TOLOWER "${language}" SWIG_MODULE_${name}_SWIG_LANGUAGE_FLAG)

  set(SWIG_MODULE_${name}_NAME "${name}")
  set(SWIG_MODULE_${name}_EXTRA_FLAGS)
  if (NOT DEFINED SWIG_MODULE_${name}_NOPROXY)
    set (SWIG_MODULE_${name}_NOPROXY FALSE)
  endif()
  if ("-noproxy" IN_LIST CMAKE_SWIG_FLAGS)
    set (SWIG_MODULE_${name}_NOPROXY TRUE)
  endif ()

  if (SWIG_MODULE_${name}_NOPROXY AND NOT "-noproxy" IN_LIST CMAKE_SWIG_FLAGS)
    list (APPEND SWIG_MODULE_${name}_EXTRA_FLAGS "-noproxy")
  endif()
  if(SWIG_MODULE_${name}_LANGUAGE STREQUAL "UNKNOWN")
    message(FATAL_ERROR "SWIG Error: Language \"${language}\" not found")
  elseif(SWIG_MODULE_${name}_LANGUAGE STREQUAL "PYTHON" AND NOT SWIG_MODULE_${name}_NOPROXY)
    # swig will produce a module.py containing an 'import _modulename' statement,
    # which implies having a corresponding _modulename.so (*NIX), _modulename.pyd (Win32),
    # unless the -noproxy flag is used
    set(SWIG_MODULE_${name}_NAME "_${name}")
  elseif(SWIG_MODULE_${name}_LANGUAGE STREQUAL "PERL")
    list(APPEND SWIG_MODULE_${name}_EXTRA_FLAGS "-shadow")
  endif()
endmacro()

#
# For a given language, input file, and output file, determine extra files that
# will be generated. This is internal swig macro.
#

function(SWIG_GET_EXTRA_OUTPUT_FILES language outfiles generatedpath infile)
  set(files)
  get_source_file_property(module_basename
    "${infile}" SWIG_MODULE_NAME)
  if(NOT swig_module_basename)

    # try to get module name from "%module foo" syntax
    if ( EXISTS "${infile}" )
      file ( STRINGS "${infile}" module_basename REGEX "[ ]*%module[ ]*[a-zA-Z0-9_]+.*" )
    endif ()
    if ( module_basename )
      string ( REGEX REPLACE "[ ]*%module[ ]*([a-zA-Z0-9_]+).*" "\\1" module_basename "${module_basename}" )

    else ()
      # try to get module name from "%module (options=...) foo" syntax
      if ( EXISTS "${infile}" )
        file ( STRINGS "${infile}" module_basename REGEX "[ ]*%module[ ]*\\(.*\\)[ ]*[a-zA-Z0-9_]+.*" )
      endif ()
      if ( module_basename )
        string ( REGEX REPLACE "[ ]*%module[ ]*\\(.*\\)[ ]*([a-zA-Z0-9_]+).*" "\\1" module_basename "${module_basename}" )

      else ()
        # fallback to file basename
        get_filename_component(module_basename "${infile}" NAME_WE)
      endif ()
    endif ()

  endif()
  foreach(it ${SWIG_${language}_EXTRA_FILE_EXTENSIONS})
    set(extra_file "${generatedpath}/${module_basename}${it}")
    list(APPEND files "${extra_file}")
  endforeach()
    # Treat extra outputs as plain files regardless of language.
  set_source_files_properties(${files} PROPERTIES LANGUAGE "")

  set (${outfiles} ${files} PARENT_SCOPE)
endfunction()

#
# Take swig (*.i) file and add proper custom commands for it
#
function(SWIG_ADD_SOURCE_TO_MODULE name outfiles infile)
  get_filename_component(swig_source_file_name_we "${infile}" NAME_WE)
  get_source_file_property(swig_source_file_cplusplus "${infile}" CPLUSPLUS)

  # If CMAKE_SWIG_OUTDIR was specified then pass it to -outdir
  if(CMAKE_SWIG_OUTDIR)
    set(outdir ${CMAKE_SWIG_OUTDIR})
  else()
    set(outdir ${CMAKE_CURRENT_BINARY_DIR})
  endif()

  if(SWIG_OUTFILE_DIR)
    set(outfiledir ${SWIG_OUTFILE_DIR})
  else()
    set(outfiledir ${outdir})
  endif()

  if(SWIG_WORKING_DIR)
    set (workingdir "${SWIG_WORKING_DIR}")
  else()
    set(workingdir "${outdir}")
  endif()

  set (swig_source_file_flags ${CMAKE_SWIG_FLAGS})
  # handle various swig compile flags properties
  get_source_file_property (include_directories "${infile}" INCLUDE_DIRECTORIES)
  if (include_directories)
    list (APPEND swig_source_file_flags "-I$<JOIN:${include_directories},$<SEMICOLON>-I>")
  endif()
  set (property "$<TARGET_PROPERTY:${name},SWIG_INCLUDE_DIRECTORIES>")
  list (APPEND swig_source_file_flags "$<$<BOOL:${property}>:-I$<JOIN:${property},$<SEMICOLON>-I>>")

  set (property "$<TARGET_PROPERTY:${name},SWIG_COMPILE_DEFINITIONS>")
  list (APPEND swig_source_file_flags "$<$<BOOL:${property}>:-D$<JOIN:${property},$<SEMICOLON>-D>>")
  get_source_file_property (compile_definitions "${infile}" COMPILE_DEFINITIONS)
  if (compile_definitions)
    list (APPEND swig_source_file_flags "-D$<JOIN:${compile_definitions},$<SEMICOLON>-D>")
  endif()

  list (APPEND swig_source_file_flags "$<TARGET_PROPERTY:${name},SWIG_COMPILE_OPTIONS>")
  get_source_file_property (compile_options "${infile}" COMPILE_OPTIONS)
  if (compile_options)
    list (APPEND swig_source_file_flags ${compile_options})
  endif()

  # legacy support
  get_source_file_property (swig_flags "${infile}" SWIG_FLAGS)
  if (swig_flags)
    list (APPEND swig_source_file_flags ${swig_flags})
  endif()

  get_filename_component(swig_source_file_fullname "${infile}" ABSOLUTE)

  if (NOT SWIG_MODULE_${name}_NOPROXY)
    SWIG_GET_EXTRA_OUTPUT_FILES(${SWIG_MODULE_${name}_LANGUAGE}
      swig_extra_generated_files
      "${outdir}"
      "${swig_source_file_fullname}")
  endif()
  set(swig_generated_file_fullname
    "${outfiledir}/${swig_source_file_name_we}")
  # add the language into the name of the file (i.e. TCL_wrap)
  # this allows for the same .i file to be wrapped into different languages
  string(APPEND swig_generated_file_fullname
    "${SWIG_MODULE_${name}_LANGUAGE}_wrap")

  if(swig_source_file_cplusplus)
    string(APPEND swig_generated_file_fullname
      ".${SWIG_CXX_EXTENSION}")
  else()
    string(APPEND swig_generated_file_fullname
      ".c")
  endif()

  get_directory_property (cmake_include_directories INCLUDE_DIRECTORIES)
  list (REMOVE_DUPLICATES cmake_include_directories)
  set (swig_include_dirs)
  if (cmake_include_directories)
    set (swig_include_dirs "-I$<JOIN:${cmake_include_directories},$<SEMICOLON>-I>")
  endif()

  set(swig_special_flags)
  # default is c, so add c++ flag if it is c++
  if(swig_source_file_cplusplus)
    list (APPEND swig_special_flags "-c++")
  endif()

  set (swig_extra_flags)
  if(SWIG_MODULE_${name}_LANGUAGE STREQUAL "CSHARP")
    if(NOT ("-dllimport" IN_LIST swig_source_file_flags OR "-dllimport" IN_LIST SWIG_MODULE_${name}_EXTRA_FLAGS))
      # This makes sure that the name used in the generated DllImport
      # matches the library name created by CMake
      list (APPEND SWIG_MODULE_${name}_EXTRA_FLAGS "-dllimport" "${name}")
    endif()
  endif()
  list (APPEND swig_extra_flags ${SWIG_MODULE_${name}_EXTRA_FLAGS})

  # dependencies
  set (swig_dependencies ${SWIG_MODULE_${name}_EXTRA_DEPS} $<TARGET_PROPERTY:${name},SWIG_DEPENDS>)
  get_source_file_property(file_depends "${infile}" DEPENDS)
  if (file_depends)
    list (APPEND swig_dependencies ${file_depends})
  endif()

  # IMPLICIT_DEPENDS below can not handle situations where a dependent file is
  # removed. We need an extra step with timestamp and custom target, see #16830
  # As this is needed only for Makefile generator do it conditionally
  if(CMAKE_GENERATOR MATCHES "Make")
    __swig_compute_timestamp(${name} ${SWIG_MODULE_${name}_LANGUAGE}
      "${infile}" "${workingdir}" swig_generated_timestamp)
    set(swig_custom_output "${swig_generated_timestamp}")
    set(swig_custom_products
      BYPRODUCTS "${swig_generated_file_fullname}" ${swig_extra_generated_files})
    set(swig_timestamp_command
      COMMAND ${CMAKE_COMMAND} -E touch "${swig_generated_timestamp}")
  else()
    set(swig_custom_output
      "${swig_generated_file_fullname}" ${swig_extra_generated_files})
    set(swig_custom_products)
    set(swig_timestamp_command)
  endif()
  add_custom_command(
    OUTPUT ${swig_custom_output}
    ${swig_custom_products}
    # Let's create the ${outdir} at execution time, in case dir contains $(OutDir)
    COMMAND "${CMAKE_COMMAND}" -E make_directory ${outdir} ${outfiledir}
    ${swig_timestamp_command}
    COMMAND "${SWIG_EXECUTABLE}"
    "-${SWIG_MODULE_${name}_SWIG_LANGUAGE_FLAG}"
    "${swig_source_file_flags}"
    -outdir "${outdir}"
    ${swig_special_flags}
    ${swig_extra_flags}
    "${swig_include_dirs}"
    -o "${swig_generated_file_fullname}"
    "${swig_source_file_fullname}"
    MAIN_DEPENDENCY "${swig_source_file_fullname}"
    DEPENDS ${swig_dependencies}
    IMPLICIT_DEPENDS CXX "${swig_source_file_fullname}"
    COMMENT "Swig source"
    COMMAND_EXPAND_LISTS)
  set_source_files_properties("${swig_generated_file_fullname}" ${swig_extra_generated_files}
    PROPERTIES GENERATED 1)

  ## add all properties for generated file to various properties
  get_property (include_directories SOURCE "${infile}" PROPERTY GENERATED_INCLUDE_DIRECTORIES)
  set_property (SOURCE "${swig_generated_file_fullname}" PROPERTY INCLUDE_DIRECTORIES ${include_directories} $<TARGET_PROPERTY:${name},SWIG_GENERATED_INCLUDE_DIRECTORIES>)

  get_property (compile_definitions SOURCE "${infile}" PROPERTY GENERATED_COMPILE_DEFINITIONS)
  set_property (SOURCE "${swig_generated_file_fullname}" PROPERTY COMPILE_DEFINITIONS $<TARGET_PROPERTY:${name},SWIG_GENERATED_COMPILE_DEFINITIONS> ${compile_definitions})

  get_property (compile_options SOURCE "${infile}" PROPERTY GENERATED_COMPILE_OPTIONS)
  set_property (SOURCE "${swig_generated_file_fullname}" PROPERTY COMPILE_OPTIONS $<TARGET_PROPERTY:${name},SWIG_GENERATED_COMPILE_OPTIONS> ${compile_options})

  set(${outfiles} "${swig_generated_file_fullname}" ${swig_extra_generated_files} PARENT_SCOPE)

  # legacy support
  set (swig_generated_file_fullname "${swig_generated_file_fullname}" PARENT_SCOPE)
endfunction()

#
# Create Swig module
#
macro(SWIG_ADD_MODULE name language)
  message(DEPRECATION "SWIG_ADD_MODULE is deprecated. Use SWIG_ADD_LIBRARY instead.")
  swig_add_library(${name}
                   LANGUAGE ${language}
                   TYPE MODULE
                   SOURCES ${ARGN})
endmacro()


function(SWIG_ADD_LIBRARY name)
  set(options NO_PROXY)
  set(oneValueArgs LANGUAGE
                   TYPE
                   OUTPUT_DIR
                   OUTFILE_DIR)
  set(multiValueArgs SOURCES)
  cmake_parse_arguments(_SAM "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if (TARGET ${name})
    # a target with same name is already defined.
    # call NOW add_library command to raise the most useful error message
    add_library(${name})
    return()
  endif()

  if (_SAM_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "SWIG_ADD_LIBRARY: ${_SAM_UNPARSED_ARGUMENTS}: unexpected arguments")
  endif()

  if(NOT DEFINED _SAM_LANGUAGE)
    message(FATAL_ERROR "SWIG_ADD_LIBRARY: Missing LANGUAGE argument")
  endif()

  if(NOT DEFINED _SAM_SOURCES)
    message(FATAL_ERROR "SWIG_ADD_LIBRARY: Missing SOURCES argument")
  endif()

  if(NOT DEFINED _SAM_TYPE)
    set(_SAM_TYPE MODULE)
  elseif(_SAM_TYPE STREQUAL "USE_BUILD_SHARED_LIBS")
    unset(_SAM_TYPE)
  endif()

  set (workingdir "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${name}.dir")
  # set special variable to pass extra information to command SWIG_ADD_SOURCE_TO_MODULE
  # which cannot be changed due to legacy compatibility
  set (SWIG_WORKING_DIR "${workingdir}")

  set (outputdir "${_SAM_OUTPUT_DIR}")
  if (NOT _SAM_OUTPUT_DIR)
    if (CMAKE_SWIG_OUTDIR)
      set (outputdir "${CMAKE_SWIG_OUTDIR}")
    else()
      set (outputdir "${CMAKE_CURRENT_BINARY_DIR}")
    endif()
  endif()

  set (outfiledir "${_SAM_OUTFILE_DIR}")
  if(NOT _SAM_OUTFILE_DIR)
    if (SWIG_OUTFILE_DIR)
      set (outfiledir "${SWIG_OUTFILE_DIR}")
    else()
      if (_SAM_OUTPUT_DIR OR CMAKE_SWIG_OUTDIR)
        set (outfiledir "${outputdir}")
    else()
        set (outfiledir "${workingdir}")
      endif()
    endif()
  endif()
  # set again, locally, predefined variables to ensure compatibility
  # with command SWIG_ADD_SOURCE_TO_MODULE
  set(CMAKE_SWIG_OUTDIR "${outputdir}")
  set(SWIG_OUTFILE_DIR "${outfiledir}")

  set (SWIG_MODULE_${name}_NOPROXY ${_SAM_NO_PROXY})
  swig_module_initialize(${name} ${_SAM_LANGUAGE})

  set(swig_dot_i_sources ${_SAM_SOURCES})
  list(FILTER swig_dot_i_sources INCLUDE REGEX "\\.i$")
  set(swig_other_sources ${_SAM_SOURCES})
  list(REMOVE_ITEM swig_other_sources ${swig_dot_i_sources})

  set(swig_generated_sources)
  set(swig_generated_timestamps)
  foreach(swig_it IN LISTS swig_dot_i_sources)
    SWIG_ADD_SOURCE_TO_MODULE(${name} swig_generated_source "${swig_it}")
    list (APPEND swig_generated_sources "${swig_generated_source}")
    if(CMAKE_GENERATOR MATCHES "Make")
      __swig_compute_timestamp(${name} ${SWIG_MODULE_${name}_LANGUAGE} "${swig_it}"
        "${workingdir}" swig_timestamp)
      list (APPEND swig_generated_timestamps "${swig_timestamp}")
    endif()
  endforeach()
  set_property (DIRECTORY APPEND PROPERTY
    ADDITIONAL_MAKE_CLEAN_FILES ${swig_generated_sources} ${swig_generated_timestamps})

  add_library(${name}
    ${_SAM_TYPE}
    ${swig_generated_sources}
    ${swig_other_sources})
  set_target_properties(${name} PROPERTIES OUTPUT_NAME "${SWIG_MODULE_${name}_NAME}")
  if(CMAKE_GENERATOR MATCHES "Make")
    # see IMPLICIT_DEPENDS above
    add_custom_target(${name}_swig_compilation DEPENDS ${swig_generated_timestamps})
    add_dependencies(${name} ${name}_swig_compilation)
  endif()
  if(_SAM_TYPE STREQUAL "MODULE")
    set_target_properties(${name} PROPERTIES NO_SONAME ON)
  endif()
  string(TOLOWER "${_SAM_LANGUAGE}" swig_lowercase_language)
  if (swig_lowercase_language STREQUAL "octave")
    set_target_properties(${name} PROPERTIES PREFIX "")
    set_target_properties(${name} PROPERTIES SUFFIX ".oct")
  elseif (swig_lowercase_language STREQUAL "go")
    set_target_properties(${name} PROPERTIES PREFIX "")
  elseif (swig_lowercase_language STREQUAL "java")
    if (APPLE)
        # In java you want:
        #      System.loadLibrary("LIBRARY");
        # then JNI will look for a library whose name is platform dependent, namely
        #   MacOS  : libLIBRARY.jnilib
        #   Windows: LIBRARY.dll
        #   Linux  : libLIBRARY.so
        set_target_properties (${name} PROPERTIES SUFFIX ".jnilib")
      endif ()
  elseif (swig_lowercase_language STREQUAL "lua")
    if(_SAM_TYPE STREQUAL "MODULE")
      set_target_properties(${name} PROPERTIES PREFIX "")
    endif()
  elseif (swig_lowercase_language STREQUAL "python")
    # this is only needed for the python case where a _modulename.so is generated
    set_target_properties(${name} PROPERTIES PREFIX "")
    # Python extension modules on Windows must have the extension ".pyd"
    # instead of ".dll" as of Python 2.5.  Older python versions do support
    # this suffix.
    # http://docs.python.org/whatsnew/ports.html#SECTION0001510000000000000000
    # <quote>
    # Windows: .dll is no longer supported as a filename extension for extension modules.
    # .pyd is now the only filename extension that will be searched for.
    # </quote>
    if(WIN32 AND NOT CYGWIN)
      set_target_properties(${name} PROPERTIES SUFFIX ".pyd")
    endif()
  elseif (swig_lowercase_language STREQUAL "r")
    set_target_properties(${name} PROPERTIES PREFIX "")
  elseif (swig_lowercase_language STREQUAL "ruby")
    # In ruby you want:
    #      require 'LIBRARY'
    # then ruby will look for a library whose name is platform dependent, namely
    #   MacOS  : LIBRARY.bundle
    #   Windows: LIBRARY.dll
    #   Linux  : LIBRARY.so
    set_target_properties (${name} PROPERTIES PREFIX "")
    if (APPLE)
      set_target_properties (${name} PROPERTIES SUFFIX ".bundle")
    endif ()
  elseif (swig_lowercase_language STREQUAL "perl")
    # assume empty prefix because we expect the module to be dynamically loaded
    set_target_properties (${name} PROPERTIES PREFIX "")
    if (APPLE)
      set_target_properties (${name} PROPERTIES SUFFIX ".dylib")
    endif ()
  else()
    # assume empty prefix because we expect the module to be dynamically loaded
    set_target_properties (${name} PROPERTIES PREFIX "")
  endif ()
  # target property SWIG_SUPPORT_FILES lists proxy support files
  if (NOT SWIG_MODULE_${name}_NOPROXY)
    string(TOUPPER "${_SAM_LANGUAGE}" swig_uppercase_language)
    foreach (swig_it IN LISTS SWIG_${swig_uppercase_language}_EXTRA_FILE_EXTENSIONS)
      set (swig_support_files ${swig_generated_sources})
      list (FILTER swig_support_files INCLUDE REGEX ".*${swig_it}$")
      set_property (TARGET ${name} APPEND PROPERTY SWIG_SUPPORT_FILES ${swig_support_files})
    endforeach()
  endif()

  # to ensure legacy behavior, export some variables
  set (SWIG_MODULE_${name}_LANGUAGE "${SWIG_MODULE_${name}_LANGUAGE}" PARENT_SCOPE)
  set (SWIG_MODULE_${name}_SWIG_LANGUAGE_FLAG "${SWIG_MODULE_${name}_SWIG_LANGUAGE_FLAG}" PARENT_SCOPE)
  set (SWIG_MODULE_${name}_REAL_NAME "${name}" PARENT_SCOPE)
  set (SWIG_MODULE_${name}_NOPROXY "${SWIG_MODULE_${name}_NOPROXY}" PARENT_SCOPE)
  set (SWIG_MODULE_${name}_EXTRA_FLAGS "${SWIG_MODULE_${name}_EXTRA_FLAGS}" PARENT_SCOPE)
  # the last one is a bit crazy but it is documented, so...
  # NOTA: works as expected if only ONE input file is specified
  set (swig_generated_file_fullname "${swig_generated_file_fullname}" PARENT_SCOPE)
endfunction()

#
# Like TARGET_LINK_LIBRARIES but for swig modules
#
function(SWIG_LINK_LIBRARIES name)
  message(DEPRECATION "SWIG_LINK_LIBRARIES is deprecated. Use TARGET_LINK_LIBRARIES instead.")
  if(SWIG_MODULE_${name}_REAL_NAME)
    target_link_libraries(${name} ${ARGN})
  else()
    message(SEND_ERROR "Cannot find Swig library \"${name}\".")
  endif()
endfunction()
