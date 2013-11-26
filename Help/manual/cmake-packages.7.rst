.. cmake-manual-description: CMake Packages Reference

cmake-packages(7)
*****************

.. only:: html or latex

   .. contents::

Introduction
============

Packages provide dependency information to CMake based buildsystems.  Packages
are found with the :command:`find_package` command.  The result of
using ``find_package`` is either a set of :prop_tgt:`IMPORTED` targets, or
a set of variables corresponding to build-relevant information.

CMake provides direct support for two forms of packages, config-file based
packages and find-module based packages.  Indirect support for pkg-config
packages is also provided via the :module:`FindPkgConfig` module.  In all
cases, the basic form of ``find_package`` is the same::

  find_package(Qt4 4.7.0 REQUIRED) # CMake provides a Qt4 find-module
  find_package(Qt5Core 5.1.0 REQUIRED) # Qt provides a Qt5 config-file.
  find_package(LibXml2 REQUIRED) # Use pkg-config via the LibXml2 find-module

In cases where it is known that a Config file is provided by upstream, and only
that should be used, the ``CONFIG`` keyword may be passed to ``find_package``::

  find_package(Qt5Core 5.1.0 CONFIG REQUIRED)
  find_package(Qt5Gui 5.1.0 CONFIG)

Similarly, the ``MODULE`` keyword requests that only a find-module be searched
for::

  find_package(Qt4 4.7.0 MODULE REQUIRED)

Specifying the type of package explicitly improves the error message shown to
the user if it is not found.

Both types of packages also support specifying components of a package, either
after the REQUIRED keyword::

  find_package(Qt5 5.1.0 CONFIG REQUIRED Widgets Xml Sql)

or as a separate COMPONENTS list::

  find_package(Qt5 5.1.0 COMPONENTS Widgets Xml Sql)

or as a separate OPTIONAL_COMPONENTS list::

  find_package(Qt5 5.1.0 COMPONENTS Widgets
                         OPTIONAL_COMPONENTS Xml Sql
  )

Handling of ``COMPONENTS`` and ``OPTIONAL_COMPONENTS`` is defined by the
package.

Config-file packages
====================

A config-file package is a set of files provided by upstreams for downstreams
to use. CMake searches in a number of locations for config file packages, as
described in the :command:`find_package` documentation.  The most simple way for
a CMake user to tell :manual:`cmake(1)` to search in a non-standard prefix for
a package is to set the ``CMAKE_PREFIX_PATH`` environment variable.

Config-file packages are provided by upstream vendors as part of development
packages, that is, they belong with the header files and any other files
provided to assist downsteams in using the package.

A set of variables which provide package status information are also set
automatically when using a config-file package.  The ``<Package>_FOUND``
variable is set to true or false, depending on whether the dependency was
found.  The ``<Package>_DIR`` cache variable is set to the location of the
package.

Usually, the upstream depends on CMake itself and can use some CMake facilities
for creating the package files. Consider an upstream which provides a single
shared library::

  project(UpstreamLib)

  set(CMAKE_INCLUDE_CURRENT_DIR ON)

  set(Upstream_VERSION 3.4.1)

  include(GenerateExportHeader)

  add_library(ClimbingStats SHARED climbingstats.cpp)
  generate_export_header(ClimbingStats)

  install(TARGETS ClimbingStats EXPORT ClimbingStatsTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
  )
  install(
    FILES
      climbingstats.h
      "${CMAKE_CURRENT_BINARY_DIR}/climbingstats_export.h"
    DESTINATION
      include
    COMPONENT
      Devel
  )

  include(CMakePackageConfigHelpers)
  write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/ClimbingStatsConfigVersion.cmake"
    VERSION ${Upstream_VERSION}
    COMPATIBILITY AnyNewerVersion
  )

  set(ConfigPackageLocation lib/cmake/ClimbingStats)
  install(EXPORT ClimbingStatsTargets
    FILE
      ClimbingStatsTargets.cmake
    NAMESPACE
      Upstream::
    DESTINATION
      ${ConfigPackageLocation}
  )
  install(
    FILES
      cmake/ClimbingStatsConfig.cmake
      "${CMAKE_CURRENT_BINARY_DIR}/ClimbingStatsConfigVersion.cmake"
    DESTINATION
      ${ConfigPackageLocation}
    COMPONENT
      Devel
  )

The :module:`CMakePackageConfigHelpers` module provides a macro for creating
a simple ``ConfigVersion.cmake`` file.  This file sets the version of the
package.  It is read by CMake when :command:`find_package` is called to
determine the compatibility with the requested version, and to set some
version-specific variables ``<Package>_VERSION``, ``<Package>_VERSION_MAJOR``,
``<Package>_VERSION_MINOR`` etc.  The :command:`install(EXPORT)` command is
used to export the targets in the ``ClimbingStatsTargets`` export-set, defined
previously by the :command:`install(TARGETS)` command. This command generates
the ``ClimbingStatsTargets.cmake`` file to contain :prop_tgt:`IMPORTED`
targets, suitable for use by downsteams and arranges to install it to
``lib/cmake/ClimbingStats``.  The generated ``ClimbingStatsConfigVersion.cmake``
and a ``cmake/ClimbingStatsConfig.cmake`` are installed to the same location,
completing the package.

A ``NAMESPACE`` with double-colons is specified when exporting the targets
for installation.  This convention of double-colons gives CMake a hint that
the name is an :prop_tgt:`IMPORTED` target when it is used by downstreams
with the :command:`target_link_libraries` command.  This way, CMake can
issue a diagnostic if the package providing it has not yet been found.

In this case, when using :command:`install(TARGETS)` the ``INCLUDES DESTINATION``
was specified.  This causes the ``IMPORTED`` targets to have their
:prop_tgt:`INTERFACE_INCLUDE_DIRECTORIES` populated with the ``include``
directory in the :variable:`CMAKE_INSTALL_PREFIX`.  When the ``IMPORTED``
target is used by downsteam, it automatically consumes the entries from
that property.

In this case, the ``ClimbingStatsConfig.cmake`` file could be as simple as::

  include("${CMAKE_CURRENT_LIST_DIR}/ClimbingStatsTargets.cmake")

As this allows downstreams to use the ``IMPORTED`` targets.  If any macros
should be provided by the ``ClimbingStats`` package, they should
be in a separate file which is installed to the same location as the
``ClimbingStatsConfig.cmake`` file, and included from there.

This can also be extended to cover dependencies::

  # ...
  add_library(ClimbingStats SHARED climbingstats.cpp)
  generate_export_header(ClimbingStats)

  find_package(Stats 2.6.4 REQUIRED)
  target_link_libraries(ClimbingStats PUBLIC Stats::Types)

As the ``Stats::Types`` target is a ``PUBLIC`` dependency of ``ClimbingStats``,
downsteams must also find the ``Stats`` package and link to the ``Stats::Types``
library.  The ``Stats`` package should be found in the ``ClimbingStatsConfig.cmake``
file to ensure this.  The ``find_dependency`` macro from the
:module:`CMakePackageConfigHelpers` helps with this by propagating
whether the package is ``REQUIRED``, or ``QUIET`` etc.  All ``REQUIRED``
dependencies of a package should be found in the ``Config.cmake`` file::

  include(CMakePackageConfigHelpers)
  find_dependency(Stats 2.6.4)

  include("${CMAKE_CURRENT_LIST_DIR}/ClimbingStatsTargets.cmake")
  include("${CMAKE_CURRENT_LIST_DIR}/ClimbingStatsMacros.cmake")

The ``find_dependency`` macro also sets ``ClimbingStats_FOUND`` to ``False`` if
the dependency is not found, along with a diagnostic that the ``ClimbingStats``
package can not be used without the ``Stats`` package.

If ``COMPONENTS`` are specified when the downstream uses :command:`find_package`,
they are listed in the ``<Package>_FIND_COMPONENTS`` variable. If a particular
component is non-optional, then the ``<Package>_FIND_REQUIRED_<comp>`` will
be true. This can be tested with logic in the Config file::

  include(CMakePackageConfigHelpers)
  find_dependency(Stats 2.6.4)

  include("${CMAKE_CURRENT_LIST_DIR}/ClimbingStatsTargets.cmake")
  include("${CMAKE_CURRENT_LIST_DIR}/ClimbingStatsMacros.cmake")

  set(_supported_components Plot Table)

  foreach(_comp ${ClimbingStats_FIND_COMPONENTS})
    if (NOT ";${_supported_components};" MATCHES _comp)
      set(ClimbingStats_FOUND False)
      set(ClimbingStats_NOTFOUND_MESSAGE "Specified unsupported component: ${_comp}")
    endif()
    include("${CMAKE_CURRENT_LIST_DIR}ClimbingStats${_comp}Targets.cmake")
  endforeach()

Here, the ``ClimbingStats_NOTFOUND_MESSAGE`` is set to a diagnosis that the package
could not be found because an invalid component was specified.  This message
variable can be set for any case where the ``_FOUND`` variable is set to ``False``,
and will be displayed to the user.

Find-module packages
====================

A find module is a file with a set of rules for finding the required pieces of
a dependency, primarily header files and libraries.  Typically, a find module
is needed when the upstream is not built with CMake, or is not CMake-aware
enough to otherwise provide a config-file package.  Unlike a config-file, it
is not shipped with upstream, but is used by downstream to find the files by
guessing locations of files with platform-specific hints.

Unlike the case of an upstream-provided config-file, no single point of
reference identifies the package as being found, so the ``<Package>_FOUND``
variable is not automatically set.  Similarly there is no ``<Package>_DIR``
variable, but each of the artifacts such as library locations and header file
locations provide a separate cache variable.

A find module may look something like::

  find_path(Pike_INCLUDE_DIR pike.h
    /usr/include/pike
    /usr/local/include/pike
  )

  find_library(PikeCore_LIBRARY
    NAMES pike-core pike-core7.4
  )
  find_library(PikeStream_LIBRARY
    NAMES pike-stream pike-stream7.4
  )

  mark_as_advanced(
    PikeCore_LIBRARY
    PikeStream_LIBRARY
    Pike_INCLUDE_DIR
  )
  set(PikeCore_LIBRARIES ${PikeCore_LIBRARY})
  set(PikeStream_LIBRARIES ${PikeStream_LIBRARY} ${PikeCore_LIBRARIES})
  set(Pike_LIBRARIES ${PikeCore_LIBRARIES} ${PikeStream_LIBRARIES})
  set(Pike_INCLUDE_DIRS ${Pike_INCLUDE_DIR})

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(Pike
    FOUND_VAR Pike_FOUND
    REQUIRED_VARS Pike_LIBRARIES Pike_INCLUDE_DIRS
  )

In this case, the :command:`find_path` command is used to try to find
the ``pike.h`` file and store the result in ``Pike_INCLUDE_DIR``. Similarly
the :command:`find_library` command is used to try to find the ``pike-core``
library with two possible names.  The result is stored as ``PikeCore_LIBRARY``,
and all cache variables are marked as advanced to hide them from the user.

After the files are found, normal variables are set.  These variables are for
use in the CMakeLists.txt.  For example::

  find_package(Pike)
  if (Pike_FOUND)
    target_include_directories(user PRIVATE ${Pike_INCLUDE_DIRS})
    target_link_libraries(user PRIVATE ${Pike_LIBRARIES})
  endif()

The non-plural variables are for cache-use only, and they should list all
dependencies of the found libraries or headers too.

The author of a find-module may alternatively choose to create an
:prop_tgt:`IMPORTED` target for the found libraries::

  find_path(Pike_INCLUDE_DIR pike.h
    /usr/include/pike
    /usr/local/include/pike
  )

  find_library(PikeCore_LIBRARY
    NAMES pike-core pike-core7.4
  )
  find_library(PikeStream_LIBRARY
    NAMES pike-stream pike-stream7.4
  )

  mark_as_advanced(
    PikeCore_LIBRARY
    PikeStream_LIBRARY
    Pike_INCLUDE_DIR
  )

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(Pike
    FOUND_VAR Pike_FOUND
    REQUIRED_VARS Pike_LIBRARIES Pike_INCLUDE_DIRS
  )

  add_library(Pike::Core SHARED IMPORTED)
  set_property(TARGET Pike::Core
    PROPERTY IMPORTED_LOCATION ${PikeCore_LIBRARY}
  )
  set_property(TARGET Pike::Core
    PROPERTY INTERFACE_INCLUDE_DIRECTORIES
      ${Pike_INCLUDE_DIR}
  )
  add_library(Pike::Stream SHARED IMPORTED)
  set_property(TARGET Pike::Stream
    PROPERTY IMPORTED_LOCATION ${PikeStream_LIBRARY}
  )
  set_property(TARGET Pike::Stream
    PROPERTY INTERFACE_LINK_LIBRARIES
      Pike::Core
  )

This way, downstreams can simply use :command:`target_link_libraries` and take
advantage of the usage-requirement specifications encoded in the ``IMPORTED``
targets::

  find_package(Pike REQUIRED)
  target_link_libraries(user PRIVATE Pike::Core)
