find_package
------------

.. |FIND_XXX| replace:: find_package
.. |FIND_ARGS_XXX| replace:: <PackageName>
.. |FIND_XXX_REGISTRY_VIEW_DEFAULT| replace:: ``TARGET``
.. |CMAKE_FIND_ROOT_PATH_MODE_XXX| replace::
   :variable:`CMAKE_FIND_ROOT_PATH_MODE_PACKAGE`

.. only:: html

   .. contents::

.. note:: The :guide:`Using Dependencies Guide` provides a high-level
  introduction to this general topic. It provides a broader overview of
  where the ``find_package()`` command fits into the bigger picture,
  including its relationship to the :module:`FetchContent` module.
  The guide is recommended pre-reading before moving on to the details below.

Find a package (usually provided by something external to the project),
and load its package-specific details.  Calls to this command can also
be intercepted by :ref:`dependency providers <dependency_providers>`.

Search Modes
^^^^^^^^^^^^

The command has a few modes by which it searches for packages:

**Module mode**
  In this mode, CMake searches for a file called ``Find<PackageName>.cmake``,
  looking first in the locations listed in the :variable:`CMAKE_MODULE_PATH`,
  then among the :ref:`Find Modules` provided by the CMake installation.
  If the file is found, it is read and processed by CMake.  It is responsible
  for finding the package, checking the version, and producing any needed
  messages.  Some Find modules provide limited or no support for versioning;
  check the Find module's documentation.

  The ``Find<PackageName>.cmake`` file is not typically provided by the
  package itself.  Rather, it is normally provided by something external to
  the package, such as the operating system, CMake itself, or even the project
  from which the ``find_package()`` command was called.  Being externally
  provided, :ref:`Find Modules` tend to be heuristic in nature and are
  susceptible to becoming out-of-date.  They typically search for certain
  libraries, files and other package artifacts.

  Module mode is only supported by the
  :ref:`basic command signature <Basic Signature>`.

**Config mode**
  In this mode, CMake searches for a file called
  ``<lowercasePackageName>-config.cmake`` or ``<PackageName>Config.cmake``.
  It will also look for ``<lowercasePackageName>-config-version.cmake`` or
  ``<PackageName>ConfigVersion.cmake`` if version details were specified
  (see :ref:`version selection` for an explanation of how these separate
  version files are used).

  In config mode, the command can be given a list of names to search for
  as package names.  The locations where CMake searches for the config and
  version files is considerably more complicated than for Module mode
  (see :ref:`search procedure`).

  The config and version files are typically installed as part of the
  package, so they tend to be more reliable than Find modules.  They usually
  contain direct knowledge of the package contents, so no searching or
  heuristics are needed within the config or version files themselves.

  Config mode is supported by both the :ref:`basic <Basic Signature>` and
  :ref:`full <Full Signature>` command signatures.

**FetchContent redirection mode**
  .. versionadded:: 3.24
    A call to ``find_package()`` can be redirected internally to a package
    provided by the :module:`FetchContent` module.  To the caller, the behavior
    will appear similar to Config mode, except that the search logic is
    by-passed and the component information is not used.  See
    :command:`FetchContent_Declare` and :command:`FetchContent_MakeAvailable`
    for further details.

When not redirected to a package provided by :module:`FetchContent`, the
command arguments determine whether Module or Config mode is used.  When the
`basic signature`_ is used, the command searches in Module mode first.
If the package is not found, the search falls back to Config mode.
A user may set the :variable:`CMAKE_FIND_PACKAGE_PREFER_CONFIG` variable
to true to reverse the priority and direct CMake to search using Config mode
first before falling back to Module mode.  The basic signature can also be
forced to use only Module mode with a ``MODULE`` keyword.  If the
`full signature`_ is used, the command only searches in Config mode.

Where possible, user code should generally look for packages using the
`basic signature`_, since that allows the package to be found with any mode.
Project maintainers wishing to provide a config package should understand
the bigger picture, as explained in :ref:`Full Signature` and all subsequent
sections on this page.

.. _`basic signature`:

Basic Signature
^^^^^^^^^^^^^^^

.. parsed-literal::

  find_package(<PackageName> [version] [EXACT] [QUIET] [MODULE]
               [REQUIRED] [[COMPONENTS] [components...]]
               [OPTIONAL_COMPONENTS components...]
               [REGISTRY_VIEW  (64|32|64_32|32_64|HOST|TARGET|BOTH)]
               [GLOBAL]
               [NO_POLICY_SCOPE]
               [BYPASS_PROVIDER])

The basic signature is supported by both Module and Config modes.
The ``MODULE`` keyword implies that only Module mode can be used to find
the package, with no fallback to Config mode.

Regardless of the mode used, a ``<PackageName>_FOUND`` variable will be
set to indicate whether the package was found.  When the package is found,
package-specific information may be provided through other variables and
:ref:`Imported Targets` documented by the package itself.  The
``QUIET`` option disables informational messages, including those indicating
that the package cannot be found if it is not ``REQUIRED``.  The ``REQUIRED``
option stops processing with an error message if the package cannot be found.

A package-specific list of required components may be listed after the
``COMPONENTS`` keyword.  If any of these components are not able to be
satisfied, the package overall is considered to be not found.  If the
``REQUIRED`` option is also present, this is treated as a fatal error,
otherwise execution still continues.  As a form of shorthand, if the
``REQUIRED`` option is present, the ``COMPONENTS`` keyword can be omitted
and the required components can be listed directly after ``REQUIRED``.

Additional optional components may be listed after
``OPTIONAL_COMPONENTS``.  If these cannot be satisfied, the package overall
can still be considered found, as long as all required components are
satisfied.

The set of available components and their meaning are defined by the
target package.  Formally, it is up to the target package how to
interpret the component information given to it, but it should follow
the expectations stated above.  For calls where no components are specified,
there is no single expected behavior and target packages should clearly
define what occurs in such cases.  Common arrangements include assuming it
should find all components, no components or some well-defined subset of the
available components.

.. versionadded:: 3.24
  The ``REGISTRY_VIEW`` keyword specifies which registry views should be
  queried. This keyword is only meaningful on ``Windows`` platforms and will
  be ignored on all others. Formally, it is up to the target package how to
  interpret the registry view information given to it.

.. versionadded:: 3.24
  Specifying the ``GLOBAL`` keyword will promote all imported targets to
  a global scope in the importing project. Alternatively, this functionality
  can be enabled by setting the :variable:`CMAKE_FIND_PACKAGE_TARGETS_GLOBAL`
  variable.

.. _FIND_PACKAGE_VERSION_FORMAT:

The ``[version]`` argument requests a version with which the package found
should be compatible. There are two possible forms in which it may be
specified:

  * A single version with the format ``major[.minor[.patch[.tweak]]]``, where
    each component is a numeric value.
  * A version range with the format ``versionMin...[<]versionMax`` where
    ``versionMin`` and ``versionMax`` have the same format and constraints
    on components being integers as the single version.  By default, both end
    points are included.  By specifying ``<``, the upper end point will be
    excluded. Version ranges are only supported with CMake 3.19 or later.

The ``EXACT`` option requests that the version be matched exactly. This option
is incompatible with the specification of a version range.

If no ``[version]`` and/or component list is given to a recursive invocation
inside a find-module, the corresponding arguments are forwarded
automatically from the outer call (including the ``EXACT`` flag for
``[version]``).  Version support is currently provided only on a
package-by-package basis (see the `Version Selection`_ section below).
When a version range is specified but the package is only designed to expect
a single version, the package will ignore the upper end point of the range and
only take the single version at the lower end of the range into account.

See the :command:`cmake_policy` command documentation for discussion
of the ``NO_POLICY_SCOPE`` option.

.. versionadded:: 3.24
  The ``BYPASS_PROVIDER`` keyword is only allowed when ``find_package()`` is
  being called by a :ref:`dependency provider <dependency_providers>`.
  It can be used by providers to call the built-in ``find_package()``
  implementation directly and prevent that call from being re-routed back to
  itself.  Future versions of CMake may detect attempts to use this keyword
  from places other than a dependency provider and halt with a fatal error.

.. _`full signature`:

Full Signature
^^^^^^^^^^^^^^

.. parsed-literal::

  find_package(<PackageName> [version] [EXACT] [QUIET]
               [REQUIRED] [[COMPONENTS] [components...]]
               [OPTIONAL_COMPONENTS components...]
               [CONFIG|NO_MODULE]
               [GLOBAL]
               [NO_POLICY_SCOPE]
               [BYPASS_PROVIDER]
               [NAMES name1 [name2 ...]]
               [CONFIGS config1 [config2 ...]]
               [HINTS path1 [path2 ... ]]
               [PATHS path1 [path2 ... ]]
               [REGISTRY_VIEW  (64|32|64_32|32_64|HOST|TARGET|BOTH)]
               [PATH_SUFFIXES suffix1 [suffix2 ...]]
               [NO_DEFAULT_PATH]
               [NO_PACKAGE_ROOT_PATH]
               [NO_CMAKE_PATH]
               [NO_CMAKE_ENVIRONMENT_PATH]
               [NO_SYSTEM_ENVIRONMENT_PATH]
               [NO_CMAKE_PACKAGE_REGISTRY]
               [NO_CMAKE_BUILDS_PATH] # Deprecated; does nothing.
               [NO_CMAKE_SYSTEM_PATH]
               [NO_CMAKE_INSTALL_PREFIX]
               [NO_CMAKE_SYSTEM_PACKAGE_REGISTRY]
               [CMAKE_FIND_ROOT_PATH_BOTH |
                ONLY_CMAKE_FIND_ROOT_PATH |
                NO_CMAKE_FIND_ROOT_PATH])

The ``CONFIG`` option, the synonymous ``NO_MODULE`` option, or the use
of options not specified in the `basic signature`_ all enforce pure Config
mode.  In pure Config mode, the command skips Module mode search and
proceeds at once with Config mode search.

Config mode search attempts to locate a configuration file provided by the
package to be found.  A cache entry called ``<PackageName>_DIR`` is created to
hold the directory containing the file.  By default, the command searches for
a package with the name ``<PackageName>``.  If the ``NAMES`` option is given,
the names following it are used instead of ``<PackageName>``.  The names are
also considered when determining whether to redirect the call to a package
provided by :module:`FetchContent`.

The command searches for a file called ``<PackageName>Config.cmake`` or
``<lowercasePackageName>-config.cmake`` for each name specified.
A replacement set of possible configuration file names may be given
using the ``CONFIGS`` option.  The :ref:`search procedure` is specified below.
Once found, any :ref:`version constraint <version selection>` is checked,
and if satisfied, the configuration file is read and processed by CMake.
Since the file is provided by the package it already knows the
location of package contents.  The full path to the configuration file
is stored in the cmake variable ``<PackageName>_CONFIG``.

All configuration files which have been considered by CMake while
searching for the package with an appropriate version are stored in the
``<PackageName>_CONSIDERED_CONFIGS`` variable, and the associated versions
in the ``<PackageName>_CONSIDERED_VERSIONS`` variable.

If the package configuration file cannot be found CMake will generate
an error describing the problem unless the ``QUIET`` argument is
specified.  If ``REQUIRED`` is specified and the package is not found a
fatal error is generated and the configure step stops executing.  If
``<PackageName>_DIR`` has been set to a directory not containing a
configuration file CMake will ignore it and search from scratch.

Package maintainers providing CMake package configuration files are
encouraged to name and install them such that the :ref:`search procedure`
outlined below will find them without requiring use of additional options.

.. _`search procedure`:

Config Mode Search Procedure
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. note::
  When Config mode is used, this search procedure is applied regardless of
  whether the :ref:`full <full signature>` or :ref:`basic <basic signature>`
  signature was given.

.. versionadded:: 3.24
  All calls to ``find_package()`` (even in Module mode) first look for a config
  package file in the :variable:`CMAKE_FIND_PACKAGE_REDIRECTS_DIR` directory.
  The :module:`FetchContent` module, or even the project itself, may write files
  to that location to redirect ``find_package()`` calls to content already
  provided by the project.  If no config package file is found in that location,
  the search proceeds with the logic described below.

CMake constructs a set of possible installation prefixes for the
package.  Under each prefix several directories are searched for a
configuration file.  The tables below show the directories searched.
Each entry is meant for installation trees following Windows (``W``), UNIX
(``U``), or Apple (``A``) conventions:

==================================================================== ==========
 Entry                                                               Convention
==================================================================== ==========
 ``<prefix>/``                                                          W
 ``<prefix>/(cmake|CMake)/``                                            W
 ``<prefix>/<name>*/``                                                  W
 ``<prefix>/<name>*/(cmake|CMake)/``                                    W
 ``<prefix>/<name>*/(cmake|CMake)/<name>*/`` [#]_                       W
 ``<prefix>/(lib/<arch>|lib*|share)/cmake/<name>*/``                    U
 ``<prefix>/(lib/<arch>|lib*|share)/<name>*/``                          U
 ``<prefix>/(lib/<arch>|lib*|share)/<name>*/(cmake|CMake)/``            U
 ``<prefix>/<name>*/(lib/<arch>|lib*|share)/cmake/<name>*/``            W/U
 ``<prefix>/<name>*/(lib/<arch>|lib*|share)/<name>*/``                  W/U
 ``<prefix>/<name>*/(lib/<arch>|lib*|share)/<name>*/(cmake|CMake)/``    W/U
==================================================================== ==========

.. [#] .. versionadded:: 3.25

On systems supporting macOS :prop_tgt:`FRAMEWORK` and :prop_tgt:`BUNDLE`, the
following directories are searched for Frameworks or Application Bundles
containing a configuration file:

=========================================================== ==========
 Entry                                                      Convention
=========================================================== ==========
 ``<prefix>/<name>.framework/Resources/``                      A
 ``<prefix>/<name>.framework/Resources/CMake/``                A
 ``<prefix>/<name>.framework/Versions/*/Resources/``           A
 ``<prefix>/<name>.framework/Versions/*/Resources/CMake/``     A
 ``<prefix>/<name>.app/Contents/Resources/``                   A
 ``<prefix>/<name>.app/Contents/Resources/CMake/``             A
=========================================================== ==========

In all cases the ``<name>`` is treated as case-insensitive and corresponds
to any of the names specified (``<PackageName>`` or names given by ``NAMES``).

Paths with ``lib/<arch>`` are enabled if the
:variable:`CMAKE_LIBRARY_ARCHITECTURE` variable is set. ``lib*`` includes one
or more of the values ``lib64``, ``lib32``, ``libx32`` or ``lib`` (searched in
that order).

* Paths with ``lib64`` are searched on 64 bit platforms if the
  :prop_gbl:`FIND_LIBRARY_USE_LIB64_PATHS` property is set to ``TRUE``.
* Paths with ``lib32`` are searched on 32 bit platforms if the
  :prop_gbl:`FIND_LIBRARY_USE_LIB32_PATHS` property is set to ``TRUE``.
* Paths with ``libx32`` are searched on platforms using the x32 ABI
  if the :prop_gbl:`FIND_LIBRARY_USE_LIBX32_PATHS` property is set to ``TRUE``.
* The ``lib`` path is always searched.

.. versionchanged:: 3.24
  On ``Windows`` platform, it is possible to include registry queries as part
  of the directories specified through ``HINTS`` and ``PATHS`` keywords, using
  a :ref:`dedicated syntax <Find Using Windows Registry>`. Such specifications
  will be ignored on all other platforms.

.. versionadded:: 3.24
  ``REGISTRY_VIEW`` can be specified to manage ``Windows`` registry queries
  specified as part of ``PATHS`` and ``HINTS``.

.. include:: FIND_XXX_REGISTRY_VIEW.txt

If ``PATH_SUFFIXES`` is specified, the suffixes are appended to each
(``W``) or (``U``) directory entry one-by-one.

This set of directories is intended to work in cooperation with
projects that provide configuration files in their installation trees.
Directories above marked with (``W``) are intended for installations on
Windows where the prefix may point at the top of an application's
installation directory.  Those marked with (``U``) are intended for
installations on UNIX platforms where the prefix is shared by multiple
packages.  This is merely a convention, so all (``W``) and (``U``) directories
are still searched on all platforms.  Directories marked with (``A``) are
intended for installations on Apple platforms.  The
:variable:`CMAKE_FIND_FRAMEWORK` and :variable:`CMAKE_FIND_APPBUNDLE`
variables determine the order of preference.

The set of installation prefixes is constructed using the following
steps.  If ``NO_DEFAULT_PATH`` is specified all ``NO_*`` options are
enabled.

1. Search prefixes unique to the current ``<PackageName>`` being found.
   See policy :policy:`CMP0074`.

   .. versionadded:: 3.12

   Specifically, search prefixes specified by the following variables,
   in order:

   a. :variable:`<PackageName>_ROOT` CMake variable,
      where ``<PackageName>`` is the case-preserved package name.

   b. :variable:`<PACKAGENAME>_ROOT` CMake variable,
      where ``<PACKAGENAME>`` is the upper-cased package name.
      See policy :policy:`CMP0144`.

      .. versionadded:: 3.27

   c. :envvar:`<PackageName>_ROOT` environment variable,
      where ``<PackageName>`` is the case-preserved package name.

   d. :envvar:`<PACKAGENAME>_ROOT` environment variable,
      where ``<PACKAGENAME>`` is the upper-cased package name.
      See policy :policy:`CMP0144`.

      .. versionadded:: 3.27

   The package root variables are maintained as a stack so if
   called from within a find module, root paths from the parent's find
   module will also be searched after paths for the current package.
   This can be skipped if ``NO_PACKAGE_ROOT_PATH`` is passed or by setting
   the :variable:`CMAKE_FIND_USE_PACKAGE_ROOT_PATH` to ``FALSE``.

2. Search paths specified in cmake-specific cache variables.  These
   are intended to be used on the command line with a :option:`-DVAR=VALUE <cmake -D>`.
   The values are interpreted as :ref:`semicolon-separated lists <CMake Language Lists>`.
   This can be skipped if ``NO_CMAKE_PATH`` is passed or by setting the
   :variable:`CMAKE_FIND_USE_CMAKE_PATH` to ``FALSE``:

   * :variable:`CMAKE_PREFIX_PATH`
   * :variable:`CMAKE_FRAMEWORK_PATH`
   * :variable:`CMAKE_APPBUNDLE_PATH`

3. Search paths specified in cmake-specific environment variables.
   These are intended to be set in the user's shell configuration,
   and therefore use the host's native path separator
   (``;`` on Windows and ``:`` on UNIX).
   This can be skipped if ``NO_CMAKE_ENVIRONMENT_PATH`` is passed or by setting
   the :variable:`CMAKE_FIND_USE_CMAKE_ENVIRONMENT_PATH` to ``FALSE``:

   * ``<PackageName>_DIR``
   * :envvar:`CMAKE_PREFIX_PATH`
   * :envvar:`CMAKE_FRAMEWORK_PATH`
   * :envvar:`CMAKE_APPBUNDLE_PATH`

4. Search paths specified by the ``HINTS`` option.  These should be paths
   computed by system introspection, such as a hint provided by the
   location of another item already found.  Hard-coded guesses should
   be specified with the ``PATHS`` option.

5. Search the standard system environment variables.  This can be
   skipped if ``NO_SYSTEM_ENVIRONMENT_PATH`` is passed  or by setting the
   :variable:`CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH` to ``FALSE``. Path entries
   ending in ``/bin`` or ``/sbin`` are automatically converted to their
   parent directories:

   * ``PATH``

6. Search paths stored in the CMake :ref:`User Package Registry`.
   This can be skipped if ``NO_CMAKE_PACKAGE_REGISTRY`` is passed or by
   setting the variable :variable:`CMAKE_FIND_USE_PACKAGE_REGISTRY`
   to ``FALSE`` or the deprecated variable
   :variable:`CMAKE_FIND_PACKAGE_NO_PACKAGE_REGISTRY` to ``TRUE``.

   See the :manual:`cmake-packages(7)` manual for details on the user
   package registry.

7. Search cmake variables defined in the Platform files for the
   current system. The searching of :variable:`CMAKE_INSTALL_PREFIX` and
   :variable:`CMAKE_STAGING_PREFIX` can be
   skipped if ``NO_CMAKE_INSTALL_PREFIX`` is passed or by setting the
   :variable:`CMAKE_FIND_USE_INSTALL_PREFIX` to ``FALSE``. All these locations
   can be skipped if ``NO_CMAKE_SYSTEM_PATH`` is passed or by setting the
   :variable:`CMAKE_FIND_USE_CMAKE_SYSTEM_PATH` to ``FALSE``:

   * :variable:`CMAKE_SYSTEM_PREFIX_PATH`
   * :variable:`CMAKE_SYSTEM_FRAMEWORK_PATH`
   * :variable:`CMAKE_SYSTEM_APPBUNDLE_PATH`

   The platform paths that these variables contain are locations that
   typically include installed software. An example being ``/usr/local`` for
   UNIX based platforms.

8. Search paths stored in the CMake :ref:`System Package Registry`.
   This can be skipped if ``NO_CMAKE_SYSTEM_PACKAGE_REGISTRY`` is passed
   or by setting the :variable:`CMAKE_FIND_USE_SYSTEM_PACKAGE_REGISTRY`
   variable to ``FALSE`` or the deprecated variable
   :variable:`CMAKE_FIND_PACKAGE_NO_SYSTEM_PACKAGE_REGISTRY` to ``TRUE``.

   See the :manual:`cmake-packages(7)` manual for details on the system
   package registry.

9. Search paths specified by the ``PATHS`` option.  These are typically
   hard-coded guesses.

The :variable:`CMAKE_IGNORE_PATH`, :variable:`CMAKE_IGNORE_PREFIX_PATH`,
:variable:`CMAKE_SYSTEM_IGNORE_PATH` and
:variable:`CMAKE_SYSTEM_IGNORE_PREFIX_PATH` variables can also cause some
of the above locations to be ignored.

.. versionadded:: 3.16
   Added the ``CMAKE_FIND_USE_<CATEGORY>`` variables to globally disable
   various search locations.

.. include:: FIND_XXX_ROOT.txt
.. include:: FIND_XXX_ORDER.txt

By default the value stored in the result variable will be the path at
which the file is found.  The :variable:`CMAKE_FIND_PACKAGE_RESOLVE_SYMLINKS`
variable may be set to ``TRUE`` before calling ``find_package`` in order
to resolve symbolic links and store the real path to the file.

Every non-REQUIRED ``find_package`` call can be disabled or made REQUIRED:

* Setting the :variable:`CMAKE_DISABLE_FIND_PACKAGE_<PackageName>` variable
  to ``TRUE`` disables the package.  This also disables redirection to a
  package provided by :module:`FetchContent`.

* Setting the :variable:`CMAKE_REQUIRE_FIND_PACKAGE_<PackageName>` variable
  to ``TRUE`` makes the package REQUIRED.

Setting both variables to ``TRUE`` simultaneously is an error.

.. _`version selection`:

Config Mode Version Selection
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. note::
  When Config mode is used, this version selection process is applied
  regardless of whether the :ref:`full <full signature>` or
  :ref:`basic <basic signature>` signature was given.

When the ``[version]`` argument is given, Config mode will only find a
version of the package that claims compatibility with the requested
version (see :ref:`format specification <FIND_PACKAGE_VERSION_FORMAT>`). If the
``EXACT`` option is given, only a version of the package claiming an exact match
of the requested version may be found.  CMake does not establish any
convention for the meaning of version numbers.  Package version
numbers are checked by "version" files provided by the packages themselves
or by :module:`FetchContent`.  For a candidate package configuration file
``<config-file>.cmake`` the corresponding version file is located next
to it and named either ``<config-file>-version.cmake`` or
``<config-file>Version.cmake``.  If no such version file is available
then the configuration file is assumed to not be compatible with any
requested version.  A basic version file containing generic version
matching code can be created using the
:module:`CMakePackageConfigHelpers` module.  When a version file
is found it is loaded to check the requested version number.  The
version file is loaded in a nested scope in which the following
variables have been defined:

``PACKAGE_FIND_NAME``
  The ``<PackageName>``
``PACKAGE_FIND_VERSION``
  Full requested version string
``PACKAGE_FIND_VERSION_MAJOR``
  Major version if requested, else 0
``PACKAGE_FIND_VERSION_MINOR``
  Minor version if requested, else 0
``PACKAGE_FIND_VERSION_PATCH``
  Patch version if requested, else 0
``PACKAGE_FIND_VERSION_TWEAK``
  Tweak version if requested, else 0
``PACKAGE_FIND_VERSION_COUNT``
  Number of version components, 0 to 4

When a version range is specified, the above version variables will hold
values based on the lower end of the version range.  This is to preserve
compatibility with packages that have not been implemented to expect version
ranges.  In addition, the version range will be described by the following
variables:

``PACKAGE_FIND_VERSION_RANGE``
  Full requested version range string
``PACKAGE_FIND_VERSION_RANGE_MIN``
  This specifies whether the lower end point of the version range should be
  included or excluded.  Currently, the only supported value for this variable
  is ``INCLUDE``.
``PACKAGE_FIND_VERSION_RANGE_MAX``
  This specifies whether the upper end point of the version range should be
  included or excluded.  The supported values for this variable are
  ``INCLUDE`` and ``EXCLUDE``.

``PACKAGE_FIND_VERSION_MIN``
  Full requested version string of the lower end point of the range
``PACKAGE_FIND_VERSION_MIN_MAJOR``
  Major version of the lower end point if requested, else 0
``PACKAGE_FIND_VERSION_MIN_MINOR``
  Minor version of the lower end point if requested, else 0
``PACKAGE_FIND_VERSION_MIN_PATCH``
  Patch version of the lower end point if requested, else 0
``PACKAGE_FIND_VERSION_MIN_TWEAK``
  Tweak version of the lower end point if requested, else 0
``PACKAGE_FIND_VERSION_MIN_COUNT``
  Number of version components of the lower end point, 0 to 4

``PACKAGE_FIND_VERSION_MAX``
  Full requested version string of the upper end point of the range
``PACKAGE_FIND_VERSION_MAX_MAJOR``
  Major version of the upper end point if requested, else 0
``PACKAGE_FIND_VERSION_MAX_MINOR``
  Minor version of the upper end point if requested, else 0
``PACKAGE_FIND_VERSION_MAX_PATCH``
  Patch version of the upper end point if requested, else 0
``PACKAGE_FIND_VERSION_MAX_TWEAK``
  Tweak version of the upper end point if requested, else 0
``PACKAGE_FIND_VERSION_MAX_COUNT``
  Number of version components of the upper end point, 0 to 4

Regardless of whether a single version or a version range is specified, the
variable ``PACKAGE_FIND_VERSION_COMPLETE`` will be defined and will hold
the full requested version string as specified.

The version file checks whether it satisfies the requested version and
sets these variables:

``PACKAGE_VERSION``
  Full provided version string
``PACKAGE_VERSION_EXACT``
  True if version is exact match
``PACKAGE_VERSION_COMPATIBLE``
  True if version is compatible
``PACKAGE_VERSION_UNSUITABLE``
  True if unsuitable as any version

These variables are checked by the ``find_package`` command to determine
whether the configuration file provides an acceptable version.  They
are not available after the ``find_package`` call returns.  If the version
is acceptable the following variables are set:

``<PackageName>_VERSION``
  Full provided version string
``<PackageName>_VERSION_MAJOR``
  Major version if provided, else 0
``<PackageName>_VERSION_MINOR``
  Minor version if provided, else 0
``<PackageName>_VERSION_PATCH``
  Patch version if provided, else 0
``<PackageName>_VERSION_TWEAK``
  Tweak version if provided, else 0
``<PackageName>_VERSION_COUNT``
  Number of version components, 0 to 4

and the corresponding package configuration file is loaded.
When multiple package configuration files are available whose version files
claim compatibility with the version requested it is unspecified which
one is chosen: unless the variable :variable:`CMAKE_FIND_PACKAGE_SORT_ORDER`
is set no attempt is made to choose a highest or closest version number.

To control the order in which ``find_package`` checks for compatibility use
the two variables :variable:`CMAKE_FIND_PACKAGE_SORT_ORDER` and
:variable:`CMAKE_FIND_PACKAGE_SORT_DIRECTION`.
For instance in order to select the highest version one can set

.. code-block:: cmake

  SET(CMAKE_FIND_PACKAGE_SORT_ORDER NATURAL)
  SET(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC)

before calling ``find_package``.

Package File Interface Variables
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

When loading a find module or package configuration file ``find_package``
defines variables to provide information about the call arguments (and
restores their original state before returning):

``CMAKE_FIND_PACKAGE_NAME``
  The ``<PackageName>`` which is searched for
``<PackageName>_FIND_REQUIRED``
  True if ``REQUIRED`` option was given
``<PackageName>_FIND_QUIETLY``
  True if ``QUIET`` option was given
``<PackageName>_FIND_REGISTRY_VIEW``
  The requested view if ``REGISTRY_VIEW`` option was given
``<PackageName>_FIND_VERSION``
  Full requested version string
``<PackageName>_FIND_VERSION_MAJOR``
  Major version if requested, else 0
``<PackageName>_FIND_VERSION_MINOR``
  Minor version if requested, else 0
``<PackageName>_FIND_VERSION_PATCH``
  Patch version if requested, else 0
``<PackageName>_FIND_VERSION_TWEAK``
  Tweak version if requested, else 0
``<PackageName>_FIND_VERSION_COUNT``
  Number of version components, 0 to 4
``<PackageName>_FIND_VERSION_EXACT``
  True if ``EXACT`` option was given
``<PackageName>_FIND_COMPONENTS``
  List of specified components (required and optional)
``<PackageName>_FIND_REQUIRED_<c>``
  True if component ``<c>`` is required,
  false if component ``<c>`` is optional

When a version range is specified, the above version variables will hold
values based on the lower end of the version range.  This is to preserve
compatibility with packages that have not been implemented to expect version
ranges.  In addition, the version range will be described by the following
variables:

``<PackageName>_FIND_VERSION_RANGE``
  Full requested version range string
``<PackageName>_FIND_VERSION_RANGE_MIN``
  This specifies whether the lower end point of the version range is
  included or excluded.  Currently, ``INCLUDE`` is the only supported value.
``<PackageName>_FIND_VERSION_RANGE_MAX``
  This specifies whether the upper end point of the version range is
  included or excluded.  The possible values for this variable are
  ``INCLUDE`` or ``EXCLUDE``.

``<PackageName>_FIND_VERSION_MIN``
  Full requested version string of the lower end point of the range
``<PackageName>_FIND_VERSION_MIN_MAJOR``
  Major version of the lower end point if requested, else 0
``<PackageName>_FIND_VERSION_MIN_MINOR``
  Minor version of the lower end point if requested, else 0
``<PackageName>_FIND_VERSION_MIN_PATCH``
  Patch version of the lower end point if requested, else 0
``<PackageName>_FIND_VERSION_MIN_TWEAK``
  Tweak version of the lower end point if requested, else 0
``<PackageName>_FIND_VERSION_MIN_COUNT``
  Number of version components of the lower end point, 0 to 4

``<PackageName>_FIND_VERSION_MAX``
  Full requested version string of the upper end point of the range
``<PackageName>_FIND_VERSION_MAX_MAJOR``
  Major version of the upper end point if requested, else 0
``<PackageName>_FIND_VERSION_MAX_MINOR``
  Minor version of the upper end point if requested, else 0
``<PackageName>_FIND_VERSION_MAX_PATCH``
  Patch version of the upper end point if requested, else 0
``<PackageName>_FIND_VERSION_MAX_TWEAK``
  Tweak version of the upper end point if requested, else 0
``<PackageName>_FIND_VERSION_MAX_COUNT``
  Number of version components of the upper end point, 0 to 4

Regardless of whether a single version or a version range is specified, the
variable ``<PackageName>_FIND_VERSION_COMPLETE`` will be defined and will hold
the full requested version string as specified.

In Module mode the loaded find module is responsible to honor the
request detailed by these variables; see the find module for details.
In Config mode ``find_package`` handles ``REQUIRED``, ``QUIET``, and
``[version]`` options automatically but leaves it to the package
configuration file to handle components in a way that makes sense
for the package.  The package configuration file may set
``<PackageName>_FOUND`` to false to tell ``find_package`` that component
requirements are not satisfied.
