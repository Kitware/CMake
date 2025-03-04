# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

include_guard(GLOBAL)

#[=======================================================================[.rst:
FeatureSummary
--------------

Functions for generating a summary of enabled/disabled features.

These functions can be used to generate a summary of enabled and disabled
packages and/or features for a build tree such as::

    -- The following features have been enabled:

     * Example, usage example

    -- The following OPTIONAL packages have been found:

     * LibXml2 (required version >= 2.4), XML library, <http://xmlsoft.org>
       Enables HTML-import in MyWordProcessor
       Enables odt-export in MyWordProcessor
     * PNG, image library, <http://www.libpng.org/pub/png/>
       Enables saving screenshots

    -- The following OPTIONAL packages have not been found:

     * Lua, the Lua scripting language, <https://www.lua.org>
       Enables macros in MyWordProcessor
     * OpenGL, Open Graphics Library

Global Properties
^^^^^^^^^^^^^^^^^

.. variable:: FeatureSummary_PKG_TYPES

  .. versionadded:: 3.8

  This global property defines a
  :ref:`semicolon-separated list <CMake Language Lists>` of package types used
  by the ``FeatureSummary`` module.

  The order in this list is important, the first package type in the list has
  the lowest importance, while the last has the highest importance.  The type of
  a package can only be changed to a type with higher importance.

  The default package types are ``RUNTIME``, ``OPTIONAL``, ``RECOMMENDED`` and
  ``REQUIRED``, with their importance ranked as
  ``RUNTIME < OPTIONAL < RECOMMENDED < REQUIRED``.

.. variable:: FeatureSummary_REQUIRED_PKG_TYPES

  .. versionadded:: 3.8

  This global property defines a
  :ref:`semicolon-separated list <CMake Language Lists>` of package types that
  are considered required.

  If one or more packages in these categories are not found, CMake will abort
  when the :command:`feature_summary()` command is called with the
  ``FATAL_ON_MISSING_REQUIRED_PACKAGES`` option enabled.

  The default value for this global property is ``REQUIRED``.

.. variable:: FeatureSummary_DEFAULT_PKG_TYPE

  .. versionadded:: 3.8

  This global property defines the default package type.

  When the :command:`feature_summary()` command is called, and the user has not
  explicitly set a type of some package, its type will be set to this value.

  This value must be one of the types defined in the
  :variable:`FeatureSummary_PKG_TYPES` global property.

  The default value for this global property is ``OPTIONAL``.

.. variable:: FeatureSummary_<TYPE>_DESCRIPTION

  .. versionadded:: 3.9

  This global property can be defined for each package ``<TYPE>`` to a string
  that will be used in the output titles of the
  :command:`feature_summary()` command.  For example::

      The following <FeatureSummary_<TYPE>_DESCRIPTION> have been found:

  If not set, default string ``<TYPE> packages`` is used.
#]=======================================================================]

get_property(_fsPkgTypeIsSet GLOBAL PROPERTY FeatureSummary_PKG_TYPES SET)
if(NOT _fsPkgTypeIsSet)
  set_property(GLOBAL PROPERTY FeatureSummary_PKG_TYPES RUNTIME OPTIONAL RECOMMENDED REQUIRED)
endif()

get_property(_fsReqPkgTypesIsSet GLOBAL PROPERTY FeatureSummary_REQUIRED_PKG_TYPES SET)
if(NOT _fsReqPkgTypesIsSet)
  set_property(GLOBAL PROPERTY FeatureSummary_REQUIRED_PKG_TYPES REQUIRED)
endif()

get_property(_fsDefaultPkgTypeIsSet GLOBAL PROPERTY FeatureSummary_DEFAULT_PKG_TYPE SET)
if(NOT _fsDefaultPkgTypeIsSet)
  set_property(GLOBAL PROPERTY FeatureSummary_DEFAULT_PKG_TYPE OPTIONAL)
endif()

#[=======================================================================[.rst:

Functions
^^^^^^^^^

#]=======================================================================]

function(_FS_GET_FEATURE_SUMMARY _property _var _includeQuiet)

  get_property(_fsPkgTypes GLOBAL PROPERTY FeatureSummary_PKG_TYPES)
  get_property(_fsDefaultPkgType GLOBAL PROPERTY FeatureSummary_DEFAULT_PKG_TYPE)

  set(_type "ANY")
  foreach(_fsPkgType ${_fsPkgTypes})
    if("${_property}" MATCHES "${_fsPkgType}_PACKAGES_(NOT_)?FOUND")
      set(_type "${_fsPkgType}")
      break()
    endif()
  endforeach()

  if("${_property}" MATCHES "PACKAGES_FOUND")
    set(_property "PACKAGES_FOUND")
  elseif("${_property}" MATCHES "PACKAGES_NOT_FOUND")
    set(_property "PACKAGES_NOT_FOUND")
  endif()


  set(_currentFeatureText "")
  get_property(_EnabledFeatures  GLOBAL PROPERTY ${_property})
  if(_EnabledFeatures)
    list(REMOVE_DUPLICATES _EnabledFeatures)
  endif()

  foreach(_currentFeature ${_EnabledFeatures})

    # does this package belong to the type we currently want to list ?
    get_property(_currentType  GLOBAL PROPERTY _CMAKE_${_currentFeature}_TYPE)
    if(NOT _currentType)
      list(FIND _fsPkgTypes "${_fsDefaultPkgType}" _defaultInPkgTypes)
      if("${_defaultInPkgTypes}" STREQUAL "-1")
        string(REGEX REPLACE ";([^;]+)$" " and \\1" _fsPkgTypes_msg "${_fsPkgTypes}")
        string(REPLACE ";" ", " _fsPkgTypes_msg "${_fsPkgTypes_msg}")
        message(FATAL_ERROR "Bad package property type ${_fsDefaultPkgType} used in global property FeatureSummary_DEFAULT_PKG_TYPE. "
                            "Valid types are ${_fsPkgTypes_msg}. "
                            "Either update FeatureSummary_DEFAULT_PKG_TYPE or add ${_fsDefaultPkgType} to the FeatureSummary_PKG_TYPES global property.")
      endif()
      set(_currentType ${_fsDefaultPkgType})
    endif()

    if("${_type}" STREQUAL ANY  OR  "${_type}" STREQUAL "${_currentType}")
      # check whether the current feature/package should be in the output depending on whether it was QUIET or not
      set(includeThisOne TRUE)
      set(_required FALSE)
      # skip QUIET packages, except if they are REQUIRED or INCLUDE_QUIET_PACKAGES has been set
      get_property(_fsReqPkgTypes GLOBAL PROPERTY FeatureSummary_REQUIRED_PKG_TYPES)
      foreach(_fsReqPkgType ${_fsReqPkgTypes})
        if("${_currentType}" STREQUAL "${_fsReqPkgType}")
          set(_required TRUE)
          break()
        endif()
      endforeach()
      if(NOT _required AND NOT _includeQuiet)
        get_property(_isQuiet  GLOBAL PROPERTY _CMAKE_${_currentFeature}_QUIET)
        if(_isQuiet)
          set(includeThisOne FALSE)
        endif()
      endif()
      get_property(_isTransitiveDepend
        GLOBAL PROPERTY _CMAKE_${_currentFeature}_TRANSITIVE_DEPENDENCY
      )
      if(_isTransitiveDepend)
        set(includeThisOne FALSE)
      endif()

      if(includeThisOne)

        string(APPEND _currentFeatureText "\n * ${_currentFeature}")
        get_property(_info  GLOBAL PROPERTY _CMAKE_${_currentFeature}_REQUIRED_VERSION)
        if(_info)
          string(APPEND _currentFeatureText " (required version ${_info})")
        endif()
        get_property(_info  GLOBAL PROPERTY _CMAKE_${_currentFeature}_DESCRIPTION)
        if(_info)
          string(APPEND _currentFeatureText ", ${_info}")
        endif()
        get_property(_info  GLOBAL PROPERTY _CMAKE_${_currentFeature}_URL)
        if(_info)
          string(APPEND _currentFeatureText ", <${_info}>")
        endif()

        get_property(_info  GLOBAL PROPERTY _CMAKE_${_currentFeature}_PURPOSE)
        foreach(_purpose ${_info})
          string(APPEND _currentFeatureText "\n   ${_purpose}")
        endforeach()

      endif()

    endif()

  endforeach()
  set(${_var} "${_currentFeatureText}" PARENT_SCOPE)
endfunction()


#[=======================================================================[.rst:
.. command:: feature_summary

  .. code-block:: cmake

    feature_summary([FILENAME <file>]
                    [APPEND]
                    [VAR <variable_name>]
                    [INCLUDE_QUIET_PACKAGES]
                    [FATAL_ON_MISSING_REQUIRED_PACKAGES]
                    [DESCRIPTION <description> | DEFAULT_DESCRIPTION]
                    [QUIET_ON_EMPTY]
                    WHAT (ALL
                         | PACKAGES_FOUND | PACKAGES_NOT_FOUND
                         | <TYPE>_PACKAGES_FOUND | <TYPE>_PACKAGES_NOT_FOUND
                         | ENABLED_FEATURES | DISABLED_FEATURES)
                   )

  This function can be used to print information about
  enabled or disabled packages and features of a project.  By default,
  only the names of the features/packages will be printed and their
  required version when one was specified.  Use
  :command:`set_package_properties()` to add more useful information, like e.g.
  a homepage URL for the respective package or their purpose in the project.

  The options are:

  ``WHAT``
    This is the only mandatory option.  It specifies what information will be
    printed:

    ``ALL``
      Print everything.
    ``ENABLED_FEATURES``
      The list of all features which are enabled.
    ``DISABLED_FEATURES``
      The list of all features which are disabled.
    ``PACKAGES_FOUND``
      The list of all packages which have been found.
    ``PACKAGES_NOT_FOUND``
      The list of all packages which have not been found.

    For each package type ``<TYPE>`` defined by the
    :variable:`FeatureSummary_PKG_TYPES` global property, the following
    information can also be used:

    ``<TYPE>_PACKAGES_FOUND``
      The list of only packages of type ``<TYPE>`` which have been found.
    ``<TYPE>_PACKAGES_NOT_FOUND``
      The list of only packages of type ``<TYPE>`` which have not been found.

    .. versionchanged:: 3.1
      The ``WHAT`` option is now a multi-value keyword, so that these values can
      be combined, with the exception of the ``ALL`` value, in order to
      customize the output.  For example:

    .. code-block:: cmake

      feature_summary(WHAT ENABLED_FEATURES DISABLED_FEATURES)

  ``FILENAME <file>``
    If this option is given, the information is printed into this file instead
    of the terminal.  Relative ``<file>`` path is interpreted as being relative
    to the current source directory (i.e. :variable:`CMAKE_CURRENT_SOURCE_DIR`).

  ``APPEND``
    If this option is given, the output is appended to the ``<file>`` provided
    by the ``FILENAME`` option, otherwise the file is overwritten if it already
    exists.

  ``VAR <variable_name>``
    If this option is given, the information is stored into the specified
    variable ``<variable_name>`` instead of the terminal.

  ``DESCRIPTION <description>``
    A description or headline which will be printed above the actual content.
    Without this option, if only one package type was requested, no title is
    printed, unless a custom string is explicitly set using this option or
    ``DEFAULT_DESCRIPTION`` option is used that outputs a default title for the
    requested type.

  ``DEFAULT_DESCRIPTION``
    .. versionadded:: 3.9

    The default description or headline to be printed above the content as
    opposed to the customizable ``DESCRIPTION <description>``.

  ``INCLUDE_QUIET_PACKAGES``
    If this option is given, packages which have been searched with
    :command:`find_package(... QUIET)` will also be listed.  By default they are
    skipped.

  ``FATAL_ON_MISSING_REQUIRED_PACKAGES``
    If this option is given, CMake will abort with fatal error if a package
    which is marked as one of the package types listed in the
    :variable:`FeatureSummary_REQUIRED_PKG_TYPES` global property has not been
    found.

  The :variable:`FeatureSummary_DEFAULT_PKG_TYPE` global property can be
  modified to change the default package type assigned when not explicitly
  assigned by the user.

  ``QUIET_ON_EMPTY``
    .. versionadded:: 3.8

    If this option is given, when only one package type was requested, and no
    packages belonging to that category were found, then no output (including
    the ``DESCRIPTION``) is printed nor added to the ``FILENAME``, or the
    ``VAR`` variable.

  Example 1, append everything to a file:

  .. code-block:: cmake

   include(FeatureSummary)
   feature_summary(WHAT ALL
                   FILENAME ${CMAKE_BINARY_DIR}/all.log APPEND)

  Example 2, print the enabled features into the variable
  ``enabledFeaturesText``, including the ``QUIET`` packages:

  .. code-block:: cmake

    include(FeatureSummary)
    feature_summary(WHAT ENABLED_FEATURES
                   INCLUDE_QUIET_PACKAGES
                   DESCRIPTION "Enabled Features:"
                   VAR enabledFeaturesText)
    message(STATUS "${enabledFeaturesText}")

  Example 3, add custom package type and print only the categories that are not
  empty:

  .. code-block:: cmake

    include(FeatureSummary)
    set_property(GLOBAL APPEND PROPERTY FeatureSummary_PKG_TYPES BUILD)
    find_package(FOO)
    set_package_properties(FOO PROPERTIES TYPE BUILD)
    feature_summary(WHAT BUILD_PACKAGES_FOUND
                    DESCRIPTION "Build tools found:"
                    QUIET_ON_EMPTY)
    feature_summary(WHAT BUILD_PACKAGES_NOT_FOUND
                    DESCRIPTION "Build tools not found:"
                    QUIET_ON_EMPTY)

#]=======================================================================]

function(FEATURE_SUMMARY)
# cmake_parse_arguments(<prefix> <options> <one_value_keywords> <multi_value_keywords> args...)
  set(options APPEND
              INCLUDE_QUIET_PACKAGES
              FATAL_ON_MISSING_REQUIRED_PACKAGES
              QUIET_ON_EMPTY
              DEFAULT_DESCRIPTION)
  set(oneValueArgs FILENAME
                   VAR
                   DESCRIPTION)
  set(multiValueArgs WHAT)

  cmake_parse_arguments(_FS "${options}" "${oneValueArgs}" "${multiValueArgs}"  ${_FIRST_ARG} ${ARGN})

  if(_FS_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown keywords given to FEATURE_SUMMARY(): \"${_FS_UNPARSED_ARGUMENTS}\"")
  endif()

  if(NOT _FS_WHAT)
    message(FATAL_ERROR "The call to FEATURE_SUMMARY() doesn't set the required WHAT argument.")
  endif()

  if(_FS_DEFAULT_DESCRIPTION AND DEFINED _FS_DESCRIPTION)
    message(WARNING "DEFAULT_DESCRIPTION option discarded since DESCRIPTION is set.")
    set(_FS_DEFAULT_DESCRIPTION 0)
  endif()

  set(validWhatParts "ENABLED_FEATURES"
                     "DISABLED_FEATURES"
                     "PACKAGES_FOUND"
                     "PACKAGES_NOT_FOUND")

  get_property(_fsPkgTypes GLOBAL PROPERTY FeatureSummary_PKG_TYPES)
  get_property(_fsReqPkgTypes GLOBAL PROPERTY FeatureSummary_REQUIRED_PKG_TYPES)
  foreach(_fsPkgType ${_fsPkgTypes})
    list(APPEND validWhatParts "${_fsPkgType}_PACKAGES_FOUND"
                               "${_fsPkgType}_PACKAGES_NOT_FOUND")
  endforeach()

  set(title_ENABLED_FEATURES               "The following features have been enabled:")
  set(title_DISABLED_FEATURES              "The following features have been disabled:")
  set(title_PACKAGES_FOUND                 "The following packages have been found:")
  set(title_PACKAGES_NOT_FOUND             "The following packages have not been found:")
  foreach(_fsPkgType ${_fsPkgTypes})
    set(_fsPkgTypeDescription "${_fsPkgType} packages")
    get_property(_fsPkgTypeDescriptionIsSet GLOBAL PROPERTY FeatureSummary_${_fsPkgType}_DESCRIPTION SET)
    if(_fsPkgTypeDescriptionIsSet)
      get_property(_fsPkgTypeDescription GLOBAL PROPERTY FeatureSummary_${_fsPkgType}_DESCRIPTION )
    endif()
    set(title_${_fsPkgType}_PACKAGES_FOUND     "The following ${_fsPkgTypeDescription} have been found:")
    set(title_${_fsPkgType}_PACKAGES_NOT_FOUND "The following ${_fsPkgTypeDescription} have not been found:")
  endforeach()

  list(FIND validWhatParts "${_FS_WHAT}" indexInList)
  if(NOT "${indexInList}" STREQUAL "-1")
    _FS_GET_FEATURE_SUMMARY( ${_FS_WHAT} _featureSummary ${_FS_INCLUDE_QUIET_PACKAGES} )
    if(_featureSummary OR NOT _FS_QUIET_ON_EMPTY)
      if(_FS_DEFAULT_DESCRIPTION)
        set(_fullText "${title_${_FS_WHAT}}\n${_featureSummary}\n")
      else()
        set(_fullText "${_FS_DESCRIPTION}${_featureSummary}\n")
      endif()
    endif()

    if(_featureSummary)
      foreach(_fsReqPkgType ${_fsReqPkgTypes})
        if("${_FS_WHAT}" STREQUAL "${_fsReqPkgType}_PACKAGES_NOT_FOUND")
          set(requiredPackagesNotFound TRUE)
          break()
        endif()
      endforeach()
    endif()

  else()
    if("${_FS_WHAT}" STREQUAL "ALL")

      set(allWhatParts "ENABLED_FEATURES")
      foreach(_fsPkgType ${_fsPkgTypes})
        list(APPEND allWhatParts "${_fsPkgType}_PACKAGES_FOUND")
      endforeach()
      list(APPEND allWhatParts "DISABLED_FEATURES")
      foreach(_fsPkgType ${_fsPkgTypes})
        list(APPEND allWhatParts "${_fsPkgType}_PACKAGES_NOT_FOUND")
      endforeach()
    else()
      set(allWhatParts)
      foreach(part ${_FS_WHAT})
        list(FIND validWhatParts "${part}" indexInList)
        if(NOT "${indexInList}" STREQUAL "-1")
          list(APPEND allWhatParts "${part}")
        else()
          if("${part}" STREQUAL "ALL")
            message(FATAL_ERROR "The WHAT argument of FEATURE_SUMMARY() contains ALL, which cannot be combined with other values.")
          else()
            message(FATAL_ERROR "The WHAT argument of FEATURE_SUMMARY() contains ${part}, which is not a valid value.")
          endif()
        endif()
      endforeach()
    endif()

    set(_fullText "${_FS_DESCRIPTION}")
    foreach(part ${allWhatParts})
      set(_tmp)
      _FS_GET_FEATURE_SUMMARY( ${part} _tmp ${_FS_INCLUDE_QUIET_PACKAGES})
      if(_tmp)
        if(_fullText)
          string(APPEND _fullText "\n-- ")
        endif()
        string(APPEND _fullText "${title_${part}}\n${_tmp}\n")
        foreach(_fsReqPkgType ${_fsReqPkgTypes})
          if("${part}" STREQUAL "${_fsReqPkgType}_PACKAGES_NOT_FOUND")
            set(requiredPackagesNotFound TRUE)
            break()
          endif()
        endforeach()
      endif()
    endforeach()
  endif()

  if(_fullText OR NOT _FS_QUIET_ON_EMPTY)
    if(_FS_FILENAME)
      if(_FS_APPEND)
        file(APPEND "${_FS_FILENAME}" "${_fullText}")
      else()
        file(WRITE  "${_FS_FILENAME}" "${_fullText}")
      endif()

    else()
      if(NOT _FS_VAR)
        message(STATUS "${_fullText}")
      endif()
    endif()

    if(_FS_VAR)
      set(${_FS_VAR} "${_fullText}" PARENT_SCOPE)
    endif()
  endif()

  if(requiredPackagesNotFound  AND  _FS_FATAL_ON_MISSING_REQUIRED_PACKAGES)
    message(FATAL_ERROR "feature_summary() Error: REQUIRED package(s) are missing, aborting CMake run.")
  endif()

endfunction()

#[=======================================================================[.rst:
.. command:: set_package_properties

  .. code-block:: cmake

    set_package_properties(<name> PROPERTIES
                           [URL <url>]
                           [DESCRIPTION <description>]
                           [TYPE (RUNTIME|OPTIONAL|RECOMMENDED|REQUIRED)]
                           [PURPOSE <purpose>]
                          )

  Use this function to configure and provide information about the package named
  ``<name>``, which can then be displayed using the
  :command:`feature_summary()` command.  This can be performed either directly
  within the corresponding :ref:`Find module <Find Modules>` or in the project
  that uses the module after invoking the :command:`find_package()` call.  The
  features for which information can be set are determined automatically after
  the :command:`find_package()` command.

  ``URL <url>``
    This should be the homepage of the package, or something similar.
    Ideally this is set already directly in the
    :ref:`Find module <Find Modules>`.

  ``DESCRIPTION <description>``
    A short description what that package is, at most one sentence.
    Ideally this is set already directly in the
    :ref:`Find module <Find Modules>`.

  ``TYPE <type>``
    What type of dependency has the using project on that package.
    Default is ``OPTIONAL``.  In this case it is a package which can be used
    by the project when available at buildtime, but it also work without.
    ``RECOMMENDED`` is similar to ``OPTIONAL``, i.e.  the project will build if
    the package is not present, but the functionality of the resulting
    binaries will be severely limited.  If a ``REQUIRED`` package is not
    available at buildtime, the project may not even build.  This can be
    combined with the
    :command:`feature_summary(FATAL_ON_MISSING_REQUIRED_PACKAGES)` command
    option.  Last, a ``RUNTIME`` package is a package which is actually not used
    at all during the build, but which is required for actually running the
    resulting binaries.  So if such a package is
    missing, the project can still be built, but it may not work later on.
    If ``set_package_properties()`` is called multiple times for the same
    package with different TYPEs, the ``TYPE`` is only changed to higher
    TYPEs (``RUNTIME < OPTIONAL < RECOMMENDED < REQUIRED``), lower TYPEs are
    ignored.  The ``TYPE`` property is project-specific, so it cannot be set
    by the :ref:`Find module <Find Modules>`, but must be set in the project.
    The accepted types can be changed by setting the
    :variable:`FeatureSummary_PKG_TYPES` global property.

  ``PURPOSE <purpose>``
    This describes which features this package enables in the
    project, i.e.  it tells the user what functionality they get in the
    resulting binaries.  If ``set_package_properties()`` is called multiple
    times for a package, all ``PURPOSE`` properties are appended to a list of
    purposes of the package in the project.  As the ``TYPE`` property, also
    the ``PURPOSE`` property is project-specific, so it cannot be set by the
    :ref:`Find module <Find Modules>`, but must be set in the project.

  Example for setting the info for a package:

  .. code-block:: cmake

    include(FeatureSummary)
    find_package(LibXml2)
    set_package_properties(LibXml2 PROPERTIES
                           DESCRIPTION "XML library"
                           URL "http://xmlsoft.org")
    # or
    set_package_properties(LibXml2 PROPERTIES
                           TYPE RECOMMENDED
                           PURPOSE "Enables HTML-import in MyWordProcessor")
    # or
    set_package_properties(LibXml2 PROPERTIES
                           TYPE OPTIONAL
                           PURPOSE "Enables odt-export in MyWordProcessor")

    find_package(DBUS)
    set_package_properties(DBUS PROPERTIES
      TYPE RUNTIME
      PURPOSE "Necessary to disable the screensaver during a presentation")
#]=======================================================================]
function(SET_PACKAGE_PROPERTIES _name _props)
  if(NOT "${_props}" STREQUAL "PROPERTIES")
    message(FATAL_ERROR "PROPERTIES keyword is missing in SET_PACKAGE_PROPERTIES() call.")
  endif()

  set(options ) # none
  set(oneValueArgs DESCRIPTION URL TYPE PURPOSE )
  set(multiValueArgs ) # none

  cmake_parse_arguments(_SPP "${options}" "${oneValueArgs}" "${multiValueArgs}"  ${ARGN})

  if(_SPP_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown keywords given to SET_PACKAGE_PROPERTIES(): \"${_SPP_UNPARSED_ARGUMENTS}\"")
  endif()

  if(_SPP_DESCRIPTION)
    get_property(_info  GLOBAL PROPERTY _CMAKE_${_name}_DESCRIPTION)
    if(_info AND NOT "${_info}" STREQUAL "${_SPP_DESCRIPTION}")
      message(STATUS "Warning: Property DESCRIPTION for package ${_name} already set to \"${_info}\", overriding it with \"${_SPP_DESCRIPTION}\"")
    endif()

    set_property(GLOBAL PROPERTY _CMAKE_${_name}_DESCRIPTION "${_SPP_DESCRIPTION}" )
  endif()


  if(_SPP_URL)
    get_property(_info  GLOBAL PROPERTY _CMAKE_${_name}_URL)
    if(_info AND NOT "${_info}" STREQUAL "${_SPP_URL}")
      message(STATUS "Warning: Property URL already set to \"${_info}\", overriding it with \"${_SPP_URL}\"")
    endif()

    set_property(GLOBAL PROPERTY _CMAKE_${_name}_URL "${_SPP_URL}" )
  endif()


  # handle the PURPOSE: use APPEND, since there can be multiple purposes for one package inside a project
  if(_SPP_PURPOSE)
    set_property(GLOBAL APPEND PROPERTY _CMAKE_${_name}_PURPOSE "${_SPP_PURPOSE}" )
  endif()

  get_property(_fsPkgTypes GLOBAL PROPERTY FeatureSummary_PKG_TYPES)
  get_property(_fsDefaultPkgType GLOBAL PROPERTY FeatureSummary_DEFAULT_PKG_TYPE)

  # handle the TYPE
  if(DEFINED _SPP_TYPE)
    # Supported types are listed in FeatureSummary_PKG_TYPES according to their priority
    get_property(_fsPkgTypes GLOBAL PROPERTY FeatureSummary_PKG_TYPES)
    list(FIND _fsPkgTypes ${_SPP_TYPE} _typeIndexInList)
    if("${_typeIndexInList}" STREQUAL "-1" )
      string(REGEX REPLACE ";([^;]+)$" " and \\1" _fsPkgTypes_msg "${_fsPkgTypes}")
      string(REPLACE ";" ", " _fsPkgTypes_msg "${_fsPkgTypes_msg}")
      message(FATAL_ERROR "Bad package property type ${_SPP_TYPE} used in SET_PACKAGE_PROPERTIES(). "
                          "Valid types are ${_fsPkgTypes_msg}." )
    endif()

    get_property(_previousType  GLOBAL PROPERTY _CMAKE_${_name}_TYPE)
    list(FIND _fsPkgTypes "${_previousType}" _prevTypeIndexInList)

    # make sure a previously set TYPE is not overridden with a lower new TYPE:
    if("${_typeIndexInList}" GREATER "${_prevTypeIndexInList}")
      set_property(GLOBAL PROPERTY _CMAKE_${_name}_TYPE "${_SPP_TYPE}" )
    endif()
  endif()

endfunction()

#[=======================================================================[.rst:
.. command:: add_feature_info

  .. code-block:: cmake

    add_feature_info(<name> <enabled> <description>)

  Use this function to add information about a feature identified with a given
  ``<name>``.  The ``<enabled>`` contains whether this feature is enabled or
  not.  It can be a variable or a list of conditions.
  ``<description>`` is a text describing the feature.  The information can
  be displayed using :command:`feature_summary()` for ``ENABLED_FEATURES`` and
  ``DISABLED_FEATURES`` respectively.

  .. versionchanged:: 3.8
    ``<enabled>`` can be a list of conditions.

  .. versionchanged:: 4.0
    Full :ref:`Condition Syntax` is now supported for ``<enabled>``.
    See policy :policy:`CMP0183`.

  Example for setting the info for a feature:

  .. code-block:: cmake

    include(FeatureSummary)

    option(WITH_FOO "Help for foo" ON)
    add_feature_info(Foo WITH_FOO "this feature provides very cool stuff")

  Example for setting feature info based on a list of conditions:

  .. code-block:: cmake

    option(WITH_FOO "Help for foo" ON)
    option(WITH_BAR "Help for bar" OFF)
    add_feature_info(
      FooBar
      "WITH_FOO;NOT WITH_BAR"
      "this feature is enabled when WITH_FOO is ON and WITH_BAR turned OFF"
    )

  Example for setting feature info depending on a full condition syntax:

  Unlike semicolon-separated list of conditions, this enables using entire
  condition syntax as being the ``if`` clause argument, such as grouping
  conditions with parens and similar.

  .. code-block:: cmake

    option(WITH_FOO "Help for foo" ON)
    option(WITH_BAR "Help for bar" ON)
    option(WITH_BAZ "Help for baz" OFF)
    add_feature_info(
      FooBarBaz
      "WITH_FOO AND (WITH_BAR OR WITH_BAZ)"
      "this feature is enabled when the entire condition is true"
    )
#]=======================================================================]
function(ADD_FEATURE_INFO _name _depends _desc)
  cmake_policy(GET CMP0183 _CDO_CMP0183
    PARENT_SCOPE # undocumented, do not use outside of CMake
  )
  set(_enabled 1)
  if("x${_CDO_CMP0183}x" STREQUAL "xNEWx")
    foreach(_d ${_depends})
      cmake_language(EVAL CODE "
        if(${_d})
        else()
          set(_enabled 0)
        endif()"
      )
    endforeach()
  else()
    foreach(_d ${_depends})
      string(REGEX REPLACE " +" ";" _d "${_d}")
      if(${_d})
      else()
        set(_enabled 0)
        break()
      endif()
    endforeach()
  endif()
  if (${_enabled})
    set_property(GLOBAL APPEND PROPERTY ENABLED_FEATURES "${_name}")
  else ()
    set_property(GLOBAL APPEND PROPERTY DISABLED_FEATURES "${_name}")
  endif ()

  set_property(GLOBAL PROPERTY _CMAKE_${_name}_DESCRIPTION "${_desc}" )

  if("x${_CDO_CMP0183}x" STREQUAL "xx" AND "x${_depends}x" MATCHES "[^A-Za-z0-9_.; ]")
    cmake_policy(GET_WARNING CMP0183 _CDO_CMP0183_WARNING)
    message(AUTHOR_WARNING "${_CDO_CMP0183_WARNING}")
  endif()
  unset(_CDO_CMP0183)
endfunction()


# The stuff below is only kept for compatibility

#[=======================================================================[.rst:
Deprecated Functions
^^^^^^^^^^^^^^^^^^^^

The following legacy and deprecated functions are provided for backward
compatibility with previous CMake versions:

.. command:: set_package_info

  .. deprecated:: 3.8

  .. code-block:: cmake

    set_package_info(<name> <description> [ <url> [<purpose>] ])

  Set up information about the package ``<name>``, which can then be displayed
  via :command:`feature_summary()`.  This can be done either directly in the
  :ref:`Find module <Find Modules>` or in the project which uses the
  ``FeatureSummary`` module after the :command:`find_package()` call.  The
  features for which information can be set are added automatically by the
  ``find_package()`` command.

  This function is deprecated.  Use the :command:`set_package_properties()`, and
  :command:`add_feature_info()` functions instead.
#]=======================================================================]
function(SET_PACKAGE_INFO _name _desc)
  message(DEPRECATION "SET_PACKAGE_INFO is deprecated. Use SET_PACKAGE_PROPERTIES instead.")
  unset(_url)
  unset(_purpose)
  if(ARGC GREATER 2)
    set(_url "${ARGV2}")
  endif()
  if(ARGC GREATER 3)
    set(_purpose "${ARGV3}")
  endif()
  set_property(GLOBAL PROPERTY _CMAKE_${_name}_DESCRIPTION "${_desc}" )
  if(NOT _url STREQUAL "")
    set_property(GLOBAL PROPERTY _CMAKE_${_name}_URL "${_url}" )
  endif()
  if(NOT _purpose STREQUAL "")
    set_property(GLOBAL APPEND PROPERTY _CMAKE_${_name}_PURPOSE "${_purpose}" )
  endif()
endfunction()

#[=======================================================================[.rst:
.. command:: set_feature_info

  .. deprecated:: 3.8

  .. code-block:: cmake

    set_feature_info(<name> <description> [<url>])

  Does the same as:

  .. code-block:: cmake

    set_package_info(<name> <description> [<url>])
#]=======================================================================]
function(SET_FEATURE_INFO)
  message(DEPRECATION "SET_FEATURE_INFO is deprecated. Use ADD_FEATURE_INFO instead.")
  set_package_info(${ARGN})
endfunction()

#[=======================================================================[.rst:
.. command:: print_enabled_features

  .. deprecated:: 3.8

  .. code-block:: cmake

    print_enabled_features()

  Does the same as:

  .. code-block:: cmake

    feature_summary(WHAT ENABLED_FEATURES DESCRIPTION "Enabled features:")
#]=======================================================================]
function(PRINT_ENABLED_FEATURES)
  message(DEPRECATION "PRINT_ENABLED_FEATURES is deprecated. Use
    feature_summary(WHAT ENABLED_FEATURES DESCRIPTION \"Enabled features:\")")
  feature_summary(WHAT ENABLED_FEATURES  DESCRIPTION "Enabled features:")
endfunction()

#[=======================================================================[.rst:
.. command:: print_disabled_features

  .. deprecated:: 3.8

  .. code-block:: cmake

    print_disabled_features()

  Does the same as:

  .. code-block:: cmake

    feature_summary(WHAT DISABLED_FEATURES DESCRIPTION "Disabled features:")
#]=======================================================================]
function(PRINT_DISABLED_FEATURES)
  message(DEPRECATION "PRINT_DISABLED_FEATURES is deprecated. Use
    feature_summary(WHAT DISABLED_FEATURES DESCRIPTION \"Disabled features:\")")
  feature_summary(WHAT DISABLED_FEATURES  DESCRIPTION "Disabled features:")
endfunction()
