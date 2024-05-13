# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
CMakePackageConfigHelpers
-------------------------

Helper functions for creating config files that can be included by other
projects to find and use a package.

Generating a Package Configuration File
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. command:: configure_package_config_file

 Create a config file for a project::

   configure_package_config_file(<input> <output>
     INSTALL_DESTINATION <path>
     [PATH_VARS <var1> <var2> ... <varN>]
     [NO_SET_AND_CHECK_MACRO]
     [NO_CHECK_REQUIRED_COMPONENTS_MACRO]
     [INSTALL_PREFIX <path>]
     )

``configure_package_config_file()`` should be used instead of the plain
:command:`configure_file()` command when creating the ``<PackageName>Config.cmake``
or ``<PackageName>-config.cmake`` file for installing a project or library.
It helps make the resulting package relocatable by avoiding hardcoded paths
in the installed ``<PackageName>Config.cmake`` file.

In a ``FooConfig.cmake`` file there may be code like this to make the install
destinations known to the using project:

.. code-block:: cmake

   set(FOO_INCLUDE_DIR   "@CMAKE_INSTALL_FULL_INCLUDEDIR@" )
   set(FOO_DATA_DIR   "@CMAKE_INSTALL_PREFIX@/@RELATIVE_DATA_INSTALL_DIR@" )
   set(FOO_ICONS_DIR   "@CMAKE_INSTALL_PREFIX@/share/icons" )
   #...logic to determine installedPrefix from the own location...
   set(FOO_CONFIG_DIR  "${installedPrefix}/@CONFIG_INSTALL_DIR@" )

All four options shown above are not sufficient  The first three hardcode the
absolute directory locations.  The fourth case works only if the logic to
determine the ``installedPrefix`` is correct, and if ``CONFIG_INSTALL_DIR``
contains a relative path, which in general cannot be guaranteed.  This has the
effect that the resulting ``FooConfig.cmake`` file would work poorly under
Windows and macOS, where users are used to choosing the install location of a
binary package at install time, independent from how
:variable:`CMAKE_INSTALL_PREFIX` was set at build/cmake time.

Using ``configure_package_config_file()`` helps.  If used correctly, it makes
the resulting ``FooConfig.cmake`` file relocatable.  Usage:

1. Write a ``FooConfig.cmake.in`` file as you are used to.
2. Insert a line at the top containing only the string ``@PACKAGE_INIT@``.
3. Instead of ``set(FOO_DIR "@SOME_INSTALL_DIR@")``, use
   ``set(FOO_DIR "@PACKAGE_SOME_INSTALL_DIR@")`` (this must be after the
   ``@PACKAGE_INIT@`` line).
4. Instead of using the normal :command:`configure_file()` command, use
   ``configure_package_config_file()``.

The ``<input>`` and ``<output>`` arguments are the input and output file, the
same way as in :command:`configure_file()`.

The ``<path>`` given to ``INSTALL_DESTINATION`` must be the destination where
the ``FooConfig.cmake`` file will be installed to.  This path can either be
absolute, or relative to the ``INSTALL_PREFIX`` path.

The variables ``<var1>`` to ``<varN>`` given as ``PATH_VARS`` are the
variables which contain install destinations.  For each of them, the macro will
create a helper variable ``PACKAGE_<var...>``.  These helper variables must be
used in the ``FooConfig.cmake.in`` file for setting the installed location.
They are calculated by ``configure_package_config_file()`` so that they are
always relative to the installed location of the package.  This works both for
relative and also for absolute locations.  For absolute locations, it works
only if the absolute location is a subdirectory of ``INSTALL_PREFIX``.

.. versionadded:: 3.1
  If the ``INSTALL_PREFIX`` argument is passed, this is used as the base path to
  calculate all the relative paths.  The ``<path>`` argument must be an absolute
  path.  If this argument is not passed, the :variable:`CMAKE_INSTALL_PREFIX`
  variable will be used instead.  The default value is good when generating a
  ``FooConfig.cmake`` file to use your package from the install tree.  When
  generating a ``FooConfig.cmake`` file to use your package from the build tree,
  this option should be used.

By default, ``configure_package_config_file()`` also generates two helper
macros, ``set_and_check()`` and ``check_required_components()``, into the
``FooConfig.cmake`` file.

``set_and_check()`` should be used instead of the normal :command:`set` command
for setting directories and file locations.  In addition to setting the
variable, it also checks that the referenced file or directory actually exists
and fails with a fatal error if it doesn't.  This ensures that the generated
``FooConfig.cmake`` file does not contain wrong references.
Add the ``NO_SET_AND_CHECK_MACRO`` option to prevent the generation of the
``set_and_check()`` macro in the ``FooConfig.cmake`` file.

``check_required_components(<PackageName>)`` should be called at the end of
the ``FooConfig.cmake`` file. This macro checks whether all requested,
non-optional components have been found, and if this is not the case, it sets
the ``Foo_FOUND`` variable to ``FALSE`` so that the package is considered to
be not found.  It does that by testing the ``Foo_<Component>_FOUND``
variables for all requested required components.  This macro should be
called even if the package doesn't provide any components to make sure
users are not specifying components erroneously.  Add the
``NO_CHECK_REQUIRED_COMPONENTS_MACRO`` option to prevent the generation of the
``check_required_components()`` macro in the ``FooConfig.cmake`` file.

See also :ref:`CMakePackageConfigHelpers Examples`.

Generating a Package Version File
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. command:: write_basic_package_version_file

 Create a version file for a project::

   write_basic_package_version_file(<filename>
     [VERSION <major.minor.patch>]
     COMPATIBILITY <AnyNewerVersion|SameMajorVersion|SameMinorVersion|ExactVersion>
     [ARCH_INDEPENDENT] )


Writes a file for use as a ``<PackageName>ConfigVersion.cmake`` file to
``<filename>``.  See the documentation of :command:`find_package()` for
details on such files.

``<filename>`` is the output filename, which should be in the build tree.
``<major.minor.patch>`` is the version number of the project to be installed.

If no ``VERSION`` is given, the :variable:`PROJECT_VERSION` variable is used.
If this hasn't been set, it errors out.

The ``COMPATIBILITY`` mode ``AnyNewerVersion`` means that the installed
package version will be considered compatible if it is newer or exactly the
same as the requested version.  This mode should be used for packages which
are fully backward compatible, also across major versions.
If ``SameMajorVersion`` is used instead, then the behavior differs from
``AnyNewerVersion`` in that the major version number must be the same as
requested, e.g.  version 2.0 will not be considered compatible if 1.0 is
requested.  This mode should be used for packages which guarantee backward
compatibility within the same major version.
If ``SameMinorVersion`` is used, the behavior is the same as
``SameMajorVersion``, but both major and minor version must be the same as
requested, e.g version 0.2 will not be compatible if 0.1 is requested.
If ``ExactVersion`` is used, then the package is only considered compatible if
the requested version matches exactly its own version number (not considering
the tweak version).  For example, version 1.2.3 of a package is only
considered compatible to requested version 1.2.3.  This mode is for packages
without compatibility guarantees.
If your project has more elaborate version matching rules, you will need to
write your own custom ``<PackageName>ConfigVersion.cmake`` file instead of
using this macro.

.. versionadded:: 3.11
  The ``SameMinorVersion`` compatibility mode.

.. versionadded:: 3.14
  If ``ARCH_INDEPENDENT`` is given, the installed package version will be
  considered compatible even if it was built for a different architecture than
  the requested architecture.  Otherwise, an architecture check will be performed,
  and the package will be considered compatible only if the architecture matches
  exactly.  For example, if the package is built for a 32-bit architecture, the
  package is only considered compatible if it is used on a 32-bit architecture,
  unless ``ARCH_INDEPENDENT`` is given, in which case the package is considered
  compatible on any architecture.

  .. note:: ``ARCH_INDEPENDENT`` is intended for header-only libraries or
    similar packages with no binaries.

.. versionadded:: 3.19
  The version file generated by ``AnyNewerVersion``, ``SameMajorVersion`` and
  ``SameMinorVersion`` arguments of ``COMPATIBILITY`` handle the version range,
  if one is specified (see :command:`find_package` command for the details).
  ``ExactVersion`` mode is incompatible with version ranges and will display an
  author warning if one is specified.

Internally, this macro executes :command:`configure_file()` to create the
resulting version file.  Depending on the ``COMPATIBILITY``, the corresponding
``BasicConfigVersion-<COMPATIBILITY>.cmake.in`` file is used.
Please note that these files are internal to CMake and you should not call
:command:`configure_file()` on them yourself, but they can be used as a starting
point to create more sophisticated custom ``<PackageName>ConfigVersion.cmake``
files.

Generating an Apple Platform Selection File
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. command:: generate_apple_platform_selection_file

  .. versionadded:: 3.29

  Create an Apple platform selection file:

  .. code-block:: cmake

    generate_apple_platform_selection_file(<filename>
      INSTALL_DESTINATION <path>
      [INSTALL_PREFIX <path>]
      [MACOS_INCLUDE_FILE <file>]
      [IOS_INCLUDE_FILE <file>]
      [IOS_SIMULATOR_INCLUDE_FILE <file>]
      [TVOS_INCLUDE_FILE <file>]
      [TVOS_SIMULATOR_INCLUDE_FILE <file>]
      [WATCHOS_INCLUDE_FILE <file>]
      [WATCHOS_SIMULATOR_INCLUDE_FILE <file>]
      [VISIONOS_INCLUDE_FILE <file>]
      [VISIONOS_SIMULATOR_INCLUDE_FILE <file>]
      [ERROR_VARIABLE <variable>]
      )

  Write a file that includes an Apple-platform-specific ``.cmake`` file,
  e.g., for use as ``<PackageName>Config.cmake``.  This can be used in
  conjunction with the ``XCFRAMEWORK_LOCATION`` argument of
  :command:`export(SETUP)` to export packages in a way that a project
  built for any Apple platform can use them.

  ``INSTALL_DESTINATION <path>``
    Path to which the generated file will be installed by the caller, e.g.,
    via :command:`install(FILES)`.  The path may be either relative to the
    ``INSTALL_PREFIX`` or absolute.

  ``INSTALL_PREFIX <path>``
    Path prefix to which the package will be installed by the caller.
    The ``<path>`` argument must be an absolute path.  If this argument
    is not passed, the :variable:`CMAKE_INSTALL_PREFIX` variable will be
    used instead.

  ``MACOS_INCLUDE_FILE <file>``
    File to include if the platform is macOS.

  ``IOS_INCLUDE_FILE <file>``
    File to include if the platform is iOS.

  ``IOS_SIMULATOR_INCLUDE_FILE <file>``
    File to include if the platform is iOS Simulator.

  ``TVOS_INCLUDE_FILE <file>``
    File to include if the platform is tvOS.

  ``TVOS_SIMULATOR_INCLUDE_FILE <file>``
    File to include if the platform is tvOS Simulator.

  ``WATCHOS_INCLUDE_FILE <file>``
    File to include if the platform is watchOS.

  ``WATCHOS_SIMULATOR_INCLUDE_FILE <file>``
    File to include if the platform is watchOS Simulator.

  ``VISIONOS_INCLUDE_FILE <file>``
    File to include if the platform is visionOS.

  ``VISIONOS_SIMULATOR_INCLUDE_FILE <file>``
    File to include if the platform is visionOS Simulator.

  ``ERROR_VARIABLE <variable>``
    If the consuming project is built for an unsupported platform,
    set ``<variable>`` to an error message.  The includer may use this
    information to pretend the package was not found.  If this option
    is not given, the default behavior is to issue a fatal error.

  If any of the optional include files is not specified, and the consuming
  project is built for its corresponding platform, the generated file will
  consider the platform to be unsupported.  The behavior is determined
  by the ``ERROR_VARIABLE`` option.

Generating an Apple Architecture Selection File
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. command:: generate_apple_architecture_selection_file

  .. versionadded:: 3.29

  Create an Apple architecture selection file:

  .. code-block:: cmake

    generate_apple_architecture_selection_file(<filename>
      INSTALL_DESTINATION <path>
      [INSTALL_PREFIX <path>]
      [SINGLE_ARCHITECTURES <arch>...
       SINGLE_ARCHITECTURE_INCLUDE_FILES <file>...]
      [UNIVERSAL_ARCHITECTURES <arch>...
       UNIVERSAL_INCLUDE_FILE <file>]
      [ERROR_VARIABLE <variable>]
      )

  Write a file that includes an Apple-architecture-specific ``.cmake`` file
  based on :variable:`CMAKE_OSX_ARCHITECTURES`, e.g., for inclusion from an
  Apple-specific ``<PackageName>Config.cmake`` file.

  ``INSTALL_DESTINATION <path>``
    Path to which the generated file will be installed by the caller, e.g.,
    via :command:`install(FILES)`.  The path may be either relative to the
    ``INSTALL_PREFIX`` or absolute.

  ``INSTALL_PREFIX <path>``
    Path prefix to which the package will be installed by the caller.
    The ``<path>`` argument must be an absolute path.  If this argument
    is not passed, the :variable:`CMAKE_INSTALL_PREFIX` variable will be
    used instead.

  ``SINGLE_ARCHITECTURES <arch>...``
    Architectures provided by entries of ``SINGLE_ARCHITECTURE_INCLUDE_FILES``.

  ``SINGLE_ARCHITECTURE_INCLUDE_FILES <file>...``
    Architecture-specific files.  One of them will be loaded
    when :variable:`CMAKE_OSX_ARCHITECTURES` contains a single
    architecture matching the corresponding entry of
    ``SINGLE_ARCHITECTURES``.

  ``UNIVERSAL_ARCHITECTURES <arch>...``
    Architectures provided by the ``UNIVERSAL_INCLUDE_FILE``.

    The list may include ``$(ARCHS_STANDARD)`` to support consumption using
    the :generator:`Xcode` generator, but the architectures should always
    be listed individually too.

  ``UNIVERSAL_INCLUDE_FILE <file>``
    A file to load when :variable:`CMAKE_OSX_ARCHITECTURES` contains
    a (non-strict) subset of the ``UNIVERSAL_ARCHITECTURES`` and
    does not match any one of the ``SINGLE_ARCHITECTURES``.

  ``ERROR_VARIABLE <variable>``
    If the consuming project is built for an unsupported architecture,
    set ``<variable>`` to an error message.  The includer may use this
    information to pretend the package was not found.  If this option
    is not given, the default behavior is to issue a fatal error.

.. _`CMakePackageConfigHelpers Examples`:

Example Generating Package Files
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Example using both the :command:`configure_package_config_file` and
:command:`write_basic_package_version_file()` commands:

.. code-block:: cmake
   :caption: ``CMakeLists.txt``

   include(GNUInstallDirs)
   set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/Foo
       CACHE PATH "Location of header files" )
   set(SYSCONFIG_INSTALL_DIR ${CMAKE_INSTALL_SYSCONFDIR}/foo
       CACHE PATH "Location of configuration files" )
   #...
   include(CMakePackageConfigHelpers)
   configure_package_config_file(FooConfig.cmake.in
     ${CMAKE_CURRENT_BINARY_DIR}/FooConfig.cmake
     INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Foo
     PATH_VARS INCLUDE_INSTALL_DIR SYSCONFIG_INSTALL_DIR)
   write_basic_package_version_file(
     ${CMAKE_CURRENT_BINARY_DIR}/FooConfigVersion.cmake
     VERSION 1.2.3
     COMPATIBILITY SameMajorVersion )
   install(FILES ${CMAKE_CURRENT_BINARY_DIR}/FooConfig.cmake
                 ${CMAKE_CURRENT_BINARY_DIR}/FooConfigVersion.cmake
           DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Foo )

.. code-block:: cmake
   :caption: ``FooConfig.cmake.in``
   :force:

   set(FOO_VERSION x.y.z)
   ...
   @PACKAGE_INIT@
   ...
   set_and_check(FOO_INCLUDE_DIR "@PACKAGE_INCLUDE_INSTALL_DIR@")
   set_and_check(FOO_SYSCONFIG_DIR "@PACKAGE_SYSCONFIG_INSTALL_DIR@")

   check_required_components(Foo)
#]=======================================================================]

include(WriteBasicConfigVersionFile)

macro(WRITE_BASIC_PACKAGE_VERSION_FILE)
  write_basic_config_version_file(${ARGN})
endmacro()

function(CONFIGURE_PACKAGE_CONFIG_FILE _inputFile _outputFile)
  set(options NO_SET_AND_CHECK_MACRO NO_CHECK_REQUIRED_COMPONENTS_MACRO)
  set(oneValueArgs INSTALL_DESTINATION INSTALL_PREFIX)
  set(multiValueArgs PATH_VARS )

  cmake_parse_arguments(CCF "${options}" "${oneValueArgs}" "${multiValueArgs}"  ${ARGN})

  if(CCF_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown keywords given to CONFIGURE_PACKAGE_CONFIG_FILE(): \"${CCF_UNPARSED_ARGUMENTS}\"")
  endif()

  if(NOT CCF_INSTALL_DESTINATION)
    message(FATAL_ERROR "No INSTALL_DESTINATION given to CONFIGURE_PACKAGE_CONFIG_FILE()")
  endif()

  if(DEFINED CCF_INSTALL_PREFIX)
    if(IS_ABSOLUTE "${CCF_INSTALL_PREFIX}")
      set(installPrefix "${CCF_INSTALL_PREFIX}")
    else()
      message(FATAL_ERROR "INSTALL_PREFIX must be an absolute path")
    endif()
  elseif(IS_ABSOLUTE "${CMAKE_INSTALL_PREFIX}")
    set(installPrefix "${CMAKE_INSTALL_PREFIX}")
  else()
    get_filename_component(installPrefix "${CMAKE_INSTALL_PREFIX}" ABSOLUTE)
  endif()

  if(IS_ABSOLUTE "${CCF_INSTALL_DESTINATION}")
    set(absInstallDir "${CCF_INSTALL_DESTINATION}")
  else()
    set(absInstallDir "${installPrefix}/${CCF_INSTALL_DESTINATION}")
  endif()

  file(RELATIVE_PATH PACKAGE_RELATIVE_PATH "${absInstallDir}" "${installPrefix}" )

  foreach(var ${CCF_PATH_VARS})
    if(NOT DEFINED ${var})
      message(FATAL_ERROR "Variable ${var} does not exist")
    else()
      if(IS_ABSOLUTE "${${var}}")
        string(REPLACE "${installPrefix}" "\${PACKAGE_PREFIX_DIR}"
                        PACKAGE_${var} "${${var}}")
      else()
        set(PACKAGE_${var} "\${PACKAGE_PREFIX_DIR}/${${var}}")
      endif()
    endif()
  endforeach()

  get_filename_component(inputFileName "${_inputFile}" NAME)

  set(PACKAGE_INIT "
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was ${inputFileName}                            ########

get_filename_component(PACKAGE_PREFIX_DIR \"\${CMAKE_CURRENT_LIST_DIR}/${PACKAGE_RELATIVE_PATH}\" ABSOLUTE)
")

  if("${absInstallDir}" MATCHES "^(/usr)?/lib(64)?/.+")
    # Handle "/usr move" symlinks created by some Linux distros.
    string(APPEND PACKAGE_INIT "
# Use original install prefix when loaded through a \"/usr move\"
# cross-prefix symbolic link such as /lib -> /usr/lib.
get_filename_component(_realCurr \"\${CMAKE_CURRENT_LIST_DIR}\" REALPATH)
get_filename_component(_realOrig \"${absInstallDir}\" REALPATH)
if(_realCurr STREQUAL _realOrig)
  set(PACKAGE_PREFIX_DIR \"${installPrefix}\")
endif()
unset(_realOrig)
unset(_realCurr)
")
  endif()

  if(NOT CCF_NO_SET_AND_CHECK_MACRO)
    string(APPEND PACKAGE_INIT "
macro(set_and_check _var _file)
  set(\${_var} \"\${_file}\")
  if(NOT EXISTS \"\${_file}\")
    message(FATAL_ERROR \"File or directory \${_file} referenced by variable \${_var} does not exist !\")
  endif()
endmacro()
")
  endif()


  if(NOT CCF_NO_CHECK_REQUIRED_COMPONENTS_MACRO)
    string(APPEND PACKAGE_INIT "
macro(check_required_components _NAME)
  foreach(comp \${\${_NAME}_FIND_COMPONENTS})
    if(NOT \${_NAME}_\${comp}_FOUND)
      if(\${_NAME}_FIND_REQUIRED_\${comp})
        set(\${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()
")
  endif()

  string(APPEND PACKAGE_INIT "
####################################################################################")

  configure_file("${_inputFile}" "${_outputFile}" @ONLY)

endfunction()

function(generate_apple_platform_selection_file _output_file)
  set(_config_file_options
    MACOS_INCLUDE_FILE
    IOS_INCLUDE_FILE
    IOS_SIMULATOR_INCLUDE_FILE
    TVOS_INCLUDE_FILE
    TVOS_SIMULATOR_INCLUDE_FILE
    WATCHOS_INCLUDE_FILE
    WATCHOS_SIMULATOR_INCLUDE_FILE
    VISIONOS_INCLUDE_FILE
    VISIONOS_SIMULATOR_INCLUDE_FILE
    )

  set(_options)
  set(_single
    INSTALL_DESTINATION
    INSTALL_PREFIX
    ${_config_file_options}
    ERROR_VARIABLE
    )
  set(_multi)
  cmake_parse_arguments(PARSE_ARGV 0 _gpsf "${_options}" "${_single}" "${_multi}")

  if(NOT _gpsf_INSTALL_DESTINATION)
    message(FATAL_ERROR "No INSTALL_DESTINATION given to generate_apple_platform_selection_file()")
  endif()
  if(_gpsf_INSTALL_PREFIX)
    set(maybe_INSTALL_PREFIX INSTALL_PREFIX ${_gpsf_INSTALL_PREFIX})
  else()
    set(maybe_INSTALL_PREFIX "")
  endif()

  if(_gpsf_ERROR_VARIABLE)
    set(_branch_INIT "set(\"${_gpsf_ERROR_VARIABLE}\" \"\")")
  else()
    set(_branch_INIT "")
  endif()

  set(_else ELSE)
  foreach(_opt IN LISTS _config_file_options _else)
    if(_gpsf_${_opt})
      set(_config_file "${_gpsf_${_opt}}")
      if(NOT IS_ABSOLUTE "${_config_file}")
        string(PREPEND _config_file [[${PACKAGE_PREFIX_DIR}/]])
      endif()
      set(_branch_${_opt} "include(\"${_config_file}\")")
    elseif(_gpsf_ERROR_VARIABLE)
      set(_branch_${_opt} "set(\"${_gpsf_ERROR_VARIABLE}\" \"Platform not supported\")")
    else()
      set(_branch_${_opt} "message(FATAL_ERROR \"Platform not supported\")")
    endif()
  endforeach()

  configure_package_config_file("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Internal/ApplePlatformSelection.cmake.in" "${_output_file}"
    INSTALL_DESTINATION "${_gpsf_INSTALL_DESTINATION}"
    ${maybe_INSTALL_PREFIX}
    NO_SET_AND_CHECK_MACRO
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
    )
endfunction()

function(generate_apple_architecture_selection_file _output_file)
  set(_options)
  set(_single
    INSTALL_DESTINATION
    INSTALL_PREFIX
    UNIVERSAL_INCLUDE_FILE
    ERROR_VARIABLE
    )
  set(_multi
    SINGLE_ARCHITECTURES
    SINGLE_ARCHITECTURE_INCLUDE_FILES
    UNIVERSAL_ARCHITECTURES
    )
  cmake_parse_arguments(PARSE_ARGV 0 _gasf "${_options}" "${_single}" "${_multi}")

  if(NOT _gasf_INSTALL_DESTINATION)
    message(FATAL_ERROR "No INSTALL_DESTINATION given to generate_apple_platform_selection_file()")
  endif()
  if(_gasf_INSTALL_PREFIX)
    set(maybe_INSTALL_PREFIX INSTALL_PREFIX ${_gasf_INSTALL_PREFIX})
  else()
    set(maybe_INSTALL_PREFIX "")
  endif()

  list(LENGTH _gasf_SINGLE_ARCHITECTURES _gasf_SINGLE_ARCHITECTURES_len)
  list(LENGTH _gasf_SINGLE_ARCHITECTURE_INCLUDE_FILES _gasf_SINGLE_ARCHITECTURE_INCLUDE_FILES_len)
  if(NOT _gasf_SINGLE_ARCHITECTURES_len EQUAL _gasf_SINGLE_ARCHITECTURE_INCLUDE_FILES_len)
    message(FATAL_ERROR "SINGLE_ARCHITECTURES and SINGLE_ARCHITECTURE_INCLUDE_FILES do not have the same number of entries.")
  endif()

  set(_branch_code "")

  if(_gasf_ERROR_VARIABLE)
    string(APPEND _branch_code
      "set(\"${_gasf_ERROR_VARIABLE}\" \"\")\n"
      )
  endif()

  string(APPEND _branch_code
    "\n"
    "if(NOT CMAKE_OSX_ARCHITECTURES)\n"
    )
  if(_gasf_ERROR_VARIABLE)
    string(APPEND _branch_code
      "  set(\"${_gasf_ERROR_VARIABLE}\" \"CMAKE_OSX_ARCHITECTURES must be explicitly set for this package\")\n"
      "  return()\n"
      )
  else()
    string(APPEND _branch_code
      "  message(FATAL_ERROR \"CMAKE_OSX_ARCHITECTURES must be explicitly set for this package\")\n"
      )
  endif()
  string(APPEND _branch_code
    "endif()\n\n"
    "set(_cmake_apple_archs \"\${CMAKE_OSX_ARCHITECTURES}\")\n"
    )
  if(NOT "${_gasf_UNIVERSAL_ARCHITECTURES}" STREQUAL "")
    string(APPEND _branch_code "list(REMOVE_ITEM _cmake_apple_archs ${_gasf_UNIVERSAL_ARCHITECTURES})\n")
  endif()
  string(APPEND _branch_code "\n")

  set(maybe_else "")

  foreach(pair IN ZIP_LISTS _gasf_SINGLE_ARCHITECTURES _gasf_SINGLE_ARCHITECTURE_INCLUDE_FILES)
    set(arch "${pair_0}")
    set(config_file "${pair_1}")
    if(NOT IS_ABSOLUTE "${config_file}")
      string(PREPEND config_file [[${PACKAGE_PREFIX_DIR}/]])
    endif()
    string(APPEND _branch_code
      "${maybe_else}if(CMAKE_OSX_ARCHITECTURES STREQUAL \"${arch}\")\n"
      "  include(\"${config_file}\")\n"
      )
    set(maybe_else else)
  endforeach()

  if(_gasf_UNIVERSAL_ARCHITECTURES AND _gasf_UNIVERSAL_INCLUDE_FILE)
    set(config_file "${_gasf_UNIVERSAL_INCLUDE_FILE}")
    if(NOT IS_ABSOLUTE "${config_file}")
      string(PREPEND config_file [[${PACKAGE_PREFIX_DIR}/]])
    endif()
    string(APPEND _branch_code
      "${maybe_else}if(NOT _cmake_apple_archs)\n"
      "  include(\"${config_file}\")\n"
      )
    set(maybe_else else)
  elseif(_gasf_UNIVERSAL_ARCHITECTURES)
    message(FATAL_ERROR "UNIVERSAL_INCLUDE_FILE requires UNIVERSAL_ARCHITECTURES")
  elseif(_gasf_UNIVERSAL_INCLUDE_FILE)
    message(FATAL_ERROR "UNIVERSAL_ARCHITECTURES requires UNIVERSAL_INCLUDE_FILE")
  endif()

  if(maybe_else)
    string(APPEND _branch_code "else()\n")
    set(_indent "  ")
  else()
    set(_indent "")
  endif()
  if(_gasf_ERROR_VARIABLE)
    string(APPEND _branch_code
      "${_indent}set(\"${_gasf_ERROR_VARIABLE}\" \"Architecture not supported\")\n"
      )
  else()
    string(APPEND _branch_code
      "${_indent}message(FATAL_ERROR \"Architecture not supported\")\n"
      )
  endif()
  if(maybe_else)
    string(APPEND _branch_code "endif()\n")
  endif()

  configure_package_config_file("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Internal/AppleArchitectureSelection.cmake.in" "${_output_file}"
    INSTALL_DESTINATION "${_gasf_INSTALL_DESTINATION}"
    ${maybe_INSTALL_PREFIX}
    NO_SET_AND_CHECK_MACRO
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
    )
endfunction()
