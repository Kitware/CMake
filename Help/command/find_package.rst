find_package
------------

Load settings for an external project.

::

  find_package(<package> [version] [EXACT] [QUIET] [MODULE]
               [REQUIRED] [[COMPONENTS] [components...]]
               [OPTIONAL_COMPONENTS components...]
               [NO_POLICY_SCOPE])

Finds and loads settings from an external project.  <package>_FOUND
will be set to indicate whether the package was found.  When the
package is found package-specific information is provided through
variables and imported targets documented by the package itself.  The
QUIET option disables messages if the package cannot be found.  The
MODULE option disables the second signature documented below.  The
REQUIRED option stops processing with an error message if the package
cannot be found.

A package-specific list of required components may be listed after the
COMPONENTS option (or after the REQUIRED option if present).
Additional optional components may be listed after
OPTIONAL_COMPONENTS.  Available components and their influence on
whether a package is considered to be found are defined by the target
package.

The [version] argument requests a version with which the package found
should be compatible (format is major[.minor[.patch[.tweak]]]).  The
EXACT option requests that the version be matched exactly.  If no
[version] and/or component list is given to a recursive invocation
inside a find-module, the corresponding arguments are forwarded
automatically from the outer call (including the EXACT flag for
[version]).  Version support is currently provided only on a
package-by-package basis (details below).

User code should generally look for packages using the above simple
signature.  The remainder of this command documentation specifies the
full command signature and details of the search process.  Project
maintainers wishing to provide a package to be found by this command
are encouraged to read on.

The command has two modes by which it searches for packages: "Module"
mode and "Config" mode.  Module mode is available when the command is
invoked with the above reduced signature.  CMake searches for a file
called "Find<package>.cmake" in the CMAKE_MODULE_PATH followed by the
CMake installation.  If the file is found, it is read and processed by
CMake.  It is responsible for finding the package, checking the
version, and producing any needed messages.  Many find-modules provide
limited or no support for versioning; check the module documentation.
If no module is found and the MODULE option is not given the command
proceeds to Config mode.

The complete Config mode command signature is:

::

  find_package(<package> [version] [EXACT] [QUIET]
               [REQUIRED] [[COMPONENTS] [components...]]
               [CONFIG|NO_MODULE]
               [NO_POLICY_SCOPE]
               [NAMES name1 [name2 ...]]
               [CONFIGS config1 [config2 ...]]
               [HINTS path1 [path2 ... ]]
               [PATHS path1 [path2 ... ]]
               [PATH_SUFFIXES suffix1 [suffix2 ...]]
               [NO_DEFAULT_PATH]
               [NO_CMAKE_ENVIRONMENT_PATH]
               [NO_CMAKE_PATH]
               [NO_SYSTEM_ENVIRONMENT_PATH]
               [NO_CMAKE_PACKAGE_REGISTRY]
               [NO_CMAKE_BUILDS_PATH]
               [NO_CMAKE_SYSTEM_PATH]
               [NO_CMAKE_SYSTEM_PACKAGE_REGISTRY]
               [CMAKE_FIND_ROOT_PATH_BOTH |
                ONLY_CMAKE_FIND_ROOT_PATH |
                NO_CMAKE_FIND_ROOT_PATH])

The CONFIG option may be used to skip Module mode explicitly and
switch to Config mode.  It is synonymous to using NO_MODULE.  Config
mode is also implied by use of options not specified in the reduced
signature.

Config mode attempts to locate a configuration file provided by the
package to be found.  A cache entry called <package>_DIR is created to
hold the directory containing the file.  By default the command
searches for a package with the name <package>.  If the NAMES option
is given the names following it are used instead of <package>.  The
command searches for a file called "<name>Config.cmake" or
"<lower-case-name>-config.cmake" for each name specified.  A
replacement set of possible configuration file names may be given
using the CONFIGS option.  The search procedure is specified below.
Once found, the configuration file is read and processed by CMake.
Since the file is provided by the package it already knows the
location of package contents.  The full path to the configuration file
is stored in the cmake variable <package>_CONFIG.

All configuration files which have been considered by CMake while
searching for an installation of the package with an appropriate
version are stored in the cmake variable <package>_CONSIDERED_CONFIGS,
the associated versions in <package>_CONSIDERED_VERSIONS.

If the package configuration file cannot be found CMake will generate
an error describing the problem unless the QUIET argument is
specified.  If REQUIRED is specified and the package is not found a
fatal error is generated and the configure step stops executing.  If
<package>_DIR has been set to a directory not containing a
configuration file CMake will ignore it and search from scratch.

When the [version] argument is given Config mode will only find a
version of the package that claims compatibility with the requested
version (format is major[.minor[.patch[.tweak]]]).  If the EXACT
option is given only a version of the package claiming an exact match
of the requested version may be found.  CMake does not establish any
convention for the meaning of version numbers.  Package version
numbers are checked by "version" files provided by the packages
themselves.  For a candidate package configuration file
"<config-file>.cmake" the corresponding version file is located next
to it and named either "<config-file>-version.cmake" or
"<config-file>Version.cmake".  If no such version file is available
then the configuration file is assumed to not be compatible with any
requested version.  A basic version file containing generic version
matching code can be created using the macro
write_basic_package_version_file(), see its documentation for more
details.  When a version file is found it is loaded to check the
requested version number.  The version file is loaded in a nested
scope in which the following variables have been defined:

::

  PACKAGE_FIND_NAME          = the <package> name
  PACKAGE_FIND_VERSION       = full requested version string
  PACKAGE_FIND_VERSION_MAJOR = major version if requested, else 0
  PACKAGE_FIND_VERSION_MINOR = minor version if requested, else 0
  PACKAGE_FIND_VERSION_PATCH = patch version if requested, else 0
  PACKAGE_FIND_VERSION_TWEAK = tweak version if requested, else 0
  PACKAGE_FIND_VERSION_COUNT = number of version components, 0 to 4

The version file checks whether it satisfies the requested version and
sets these variables:

::

  PACKAGE_VERSION            = full provided version string
  PACKAGE_VERSION_EXACT      = true if version is exact match
  PACKAGE_VERSION_COMPATIBLE = true if version is compatible
  PACKAGE_VERSION_UNSUITABLE = true if unsuitable as any version

These variables are checked by the find_package command to determine
whether the configuration file provides an acceptable version.  They
are not available after the find_package call returns.  If the version
is acceptable the following variables are set:

::

  <package>_VERSION       = full provided version string
  <package>_VERSION_MAJOR = major version if provided, else 0
  <package>_VERSION_MINOR = minor version if provided, else 0
  <package>_VERSION_PATCH = patch version if provided, else 0
  <package>_VERSION_TWEAK = tweak version if provided, else 0
  <package>_VERSION_COUNT = number of version components, 0 to 4

and the corresponding package configuration file is loaded.  When
multiple package configuration files are available whose version files
claim compatibility with the version requested it is unspecified which
one is chosen.  No attempt is made to choose a highest or closest
version number.

Config mode provides an elaborate interface and search procedure.
Much of the interface is provided for completeness and for use
internally by find-modules loaded by Module mode.  Most user code
should simply call

::

  find_package(<package> [major[.minor]] [EXACT] [REQUIRED|QUIET])

in order to find a package.  Package maintainers providing CMake
package configuration files are encouraged to name and install them
such that the procedure outlined below will find them without
requiring use of additional options.

CMake constructs a set of possible installation prefixes for the
package.  Under each prefix several directories are searched for a
configuration file.  The tables below show the directories searched.
Each entry is meant for installation trees following Windows (W), UNIX
(U), or Apple (A) conventions.

::

  <prefix>/                                               (W)
  <prefix>/(cmake|CMake)/                                 (W)
  <prefix>/<name>*/                                       (W)
  <prefix>/<name>*/(cmake|CMake)/                         (W)
  <prefix>/(lib/<arch>|lib|share)/cmake/<name>*/          (U)
  <prefix>/(lib/<arch>|lib|share)/<name>*/                (U)
  <prefix>/(lib/<arch>|lib|share)/<name>*/(cmake|CMake)/  (U)

On systems supporting OS X Frameworks and Application Bundles the
following directories are searched for frameworks or bundles
containing a configuration file:

::

  <prefix>/<name>.framework/Resources/                    (A)
  <prefix>/<name>.framework/Resources/CMake/              (A)
  <prefix>/<name>.framework/Versions/*/Resources/         (A)
  <prefix>/<name>.framework/Versions/*/Resources/CMake/   (A)
  <prefix>/<name>.app/Contents/Resources/                 (A)
  <prefix>/<name>.app/Contents/Resources/CMake/           (A)

In all cases the <name> is treated as case-insensitive and corresponds
to any of the names specified (<package> or names given by NAMES).
Paths with lib/<arch> are enabled if CMAKE_LIBRARY_ARCHITECTURE is
set.  If PATH_SUFFIXES is specified the suffixes are appended to each
(W) or (U) directory entry one-by-one.

This set of directories is intended to work in cooperation with
projects that provide configuration files in their installation trees.
Directories above marked with (W) are intended for installations on
Windows where the prefix may point at the top of an application's
installation directory.  Those marked with (U) are intended for
installations on UNIX platforms where the prefix is shared by multiple
packages.  This is merely a convention, so all (W) and (U) directories
are still searched on all platforms.  Directories marked with (A) are
intended for installations on Apple platforms.  The cmake variables
CMAKE_FIND_FRAMEWORK and CMAKE_FIND_APPBUNDLE determine the order of
preference as specified below.

The set of installation prefixes is constructed using the following
steps.  If NO_DEFAULT_PATH is specified all NO_* options are enabled.

1.  Search paths specified in cmake-specific cache variables.  These
are intended to be used on the command line with a -DVAR=value.  This
can be skipped if NO_CMAKE_PATH is passed.

::

   CMAKE_PREFIX_PATH
   CMAKE_FRAMEWORK_PATH
   CMAKE_APPBUNDLE_PATH

2.  Search paths specified in cmake-specific environment variables.
These are intended to be set in the user's shell configuration.  This
can be skipped if NO_CMAKE_ENVIRONMENT_PATH is passed.

::

   <package>_DIR
   CMAKE_PREFIX_PATH
   CMAKE_FRAMEWORK_PATH
   CMAKE_APPBUNDLE_PATH

3.  Search paths specified by the HINTS option.  These should be paths
computed by system introspection, such as a hint provided by the
location of another item already found.  Hard-coded guesses should be
specified with the PATHS option.

4.  Search the standard system environment variables.  This can be
skipped if NO_SYSTEM_ENVIRONMENT_PATH is passed.  Path entries ending
in "/bin" or "/sbin" are automatically converted to their parent
directories.

::

   PATH

5.  Search project build trees recently configured in a CMake GUI.
This can be skipped if NO_CMAKE_BUILDS_PATH is passed.  It is intended
for the case when a user is building multiple dependent projects one
after another.

6.  Search paths stored in the CMake user package registry.  This can
be skipped if NO_CMAKE_PACKAGE_REGISTRY is passed.  On Windows a
<package> may appear under registry key

::

  HKEY_CURRENT_USER\Software\Kitware\CMake\Packages\<package>

as a REG_SZ value, with arbitrary name, that specifies the directory
containing the package configuration file.  On UNIX platforms a
<package> may appear under the directory

::

  ~/.cmake/packages/<package>

as a file, with arbitrary name, whose content specifies the directory
containing the package configuration file.  See the export(PACKAGE)
command to create user package registry entries for project build
trees.

7.  Search cmake variables defined in the Platform files for the
current system.  This can be skipped if NO_CMAKE_SYSTEM_PATH is
passed.

::

   CMAKE_SYSTEM_PREFIX_PATH
   CMAKE_SYSTEM_FRAMEWORK_PATH
   CMAKE_SYSTEM_APPBUNDLE_PATH

8.  Search paths stored in the CMake system package registry.  This
can be skipped if NO_CMAKE_SYSTEM_PACKAGE_REGISTRY is passed.  On
Windows a <package> may appear under registry key

::

  HKEY_LOCAL_MACHINE\Software\Kitware\CMake\Packages\<package>

as a REG_SZ value, with arbitrary name, that specifies the directory
containing the package configuration file.  There is no system package
registry on non-Windows platforms.

9.  Search paths specified by the PATHS option.  These are typically
hard-coded guesses.

On Darwin or systems supporting OS X Frameworks, the cmake variable
CMAKE_FIND_FRAMEWORK can be set to empty or one of the following:

::

   "FIRST"  - Try to find frameworks before standard
              libraries or headers. This is the default on Darwin.
   "LAST"   - Try to find frameworks after standard
              libraries or headers.
   "ONLY"   - Only try to find frameworks.
   "NEVER" - Never try to find frameworks.

On Darwin or systems supporting OS X Application Bundles, the cmake
variable CMAKE_FIND_APPBUNDLE can be set to empty or one of the
following:

::

   "FIRST"  - Try to find application bundles before standard
              programs. This is the default on Darwin.
   "LAST"   - Try to find application bundles after standard
              programs.
   "ONLY"   - Only try to find application bundles.
   "NEVER" - Never try to find application bundles.

The CMake variable CMAKE_FIND_ROOT_PATH specifies one or more
directories to be prepended to all other search directories.  This
effectively "re-roots" the entire search under given locations.  By
default it is empty.  It is especially useful when cross-compiling to
point to the root directory of the target environment and CMake will
search there too.  By default at first the directories listed in
CMAKE_FIND_ROOT_PATH and then the non-rooted directories will be
searched.  The default behavior can be adjusted by setting
CMAKE_FIND_ROOT_PATH_MODE_PACKAGE.  This behavior can be manually
overridden on a per-call basis.  By using CMAKE_FIND_ROOT_PATH_BOTH
the search order will be as described above.  If
NO_CMAKE_FIND_ROOT_PATH is used then CMAKE_FIND_ROOT_PATH will not be
used.  If ONLY_CMAKE_FIND_ROOT_PATH is used then only the re-rooted
directories will be searched.

The default search order is designed to be most-specific to
least-specific for common use cases.  Projects may override the order
by simply calling the command multiple times and using the NO_*
options:

::

   find_package(<package> PATHS paths... NO_DEFAULT_PATH)
   find_package(<package>)

Once one of the calls succeeds the result variable will be set and
stored in the cache so that no call will search again.

Every non-REQUIRED find_package() call can be disabled by setting the
variable CMAKE_DISABLE_FIND_PACKAGE_<package> to TRUE.  See the
documentation for the CMAKE_DISABLE_FIND_PACKAGE_<package> variable
for more information.

When loading a find module or package configuration file find_package
defines variables to provide information about the call arguments (and
restores their original state before returning):

::

 <package>_FIND_REQUIRED      = true if REQUIRED option was given
 <package>_FIND_QUIETLY       = true if QUIET option was given
 <package>_FIND_VERSION       = full requested version string
 <package>_FIND_VERSION_MAJOR = major version if requested, else 0
 <package>_FIND_VERSION_MINOR = minor version if requested, else 0
 <package>_FIND_VERSION_PATCH = patch version if requested, else 0
 <package>_FIND_VERSION_TWEAK = tweak version if requested, else 0
 <package>_FIND_VERSION_COUNT = number of version components, 0 to 4
 <package>_FIND_VERSION_EXACT = true if EXACT option was given
 <package>_FIND_COMPONENTS    = list of requested components
 <package>_FIND_REQUIRED_<c>  = true if component <c> is required
                                false if component <c> is optional

In Module mode the loaded find module is responsible to honor the
request detailed by these variables; see the find module for details.
In Config mode find_package handles REQUIRED, QUIET, and version
options automatically but leaves it to the package configuration file
to handle components in a way that makes sense for the package.  The
package configuration file may set <package>_FOUND to false to tell
find_package that component requirements are not satisfied.

See the cmake_policy() command documentation for discussion of the
NO_POLICY_SCOPE option.
