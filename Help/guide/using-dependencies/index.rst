Using Dependencies Guide
************************

.. only:: html

   .. contents::

Introduction
============

Projects will frequently depend on other projects, assets, and artifacts.
CMake provides a number of ways to incorporate such things into the build.
Projects and users have the flexibility to choose between methods that
best suit their needs.

The primary methods of bringing dependencies into the build are the
:command:`find_package` command and the :module:`FetchContent` module.
The :module:`FindPkgConfig` module is also sometimes used, although it
lacks some of the integration of the other two and is not discussed any
further in this guide.

Dependencies can also be made available by a custom
:ref:`dependency provider <dependency_providers>`.
This might be a third party package manager, or it might be custom code
implemented by the developer.  Dependency providers co-operate with the
primary methods mentioned above to extend their flexibility.

.. _prebuilt_find_package:

Using Pre-built Packages With ``find_package()``
================================================

A package needed by the project may already be built and available at some
location on the user's system.  That package might have also been built by
CMake, or it could have used a different build system entirely.  It might
even just be a collection of files that didn't need to be built at all.
CMake provides the :command:`find_package` command for these scenarios.
It searches well-known locations, along with additional hints and paths
provided by the project or user.  It also supports package components and
packages being optional.  Result variables are provided to allow the project
to customize its own behavior according to whether the package or specific
components were found.

In most cases, projects should generally use the :ref:`basic signature`.
Most of the time, this will involve just the package name, maybe a version
constraint, and the ``REQUIRED`` keyword if the dependency is not optional.
A set of package components may also be specified.

.. code-block:: cmake
  :caption: Examples of ``find_package()`` basic signature

  find_package(Catch2)
  find_package(GTest REQUIRED)
  find_package(Boost 1.79 COMPONENTS date_time)

The :command:`find_package` command supports two main methods for carrying
out the search:

**Config mode**
  With this method, the command looks for files that are typically provided
  by the package itself.  This is the more reliable method of the two, since
  the package details should always be in sync with the package.

**Module mode**
  Not all packages are CMake-aware. Many don't provide the files needed to
  support config mode.  For such cases, a Find module file can be provided
  separately, either by the project or by CMake.  A Find module is typically
  a heuristic implementation which knows what the package normally provides
  and how to present that package to the project.  Since Find modules are
  usually distributed separately from the package, they are not as reliable.
  They are typically maintained separately, and they are likely to follow
  different release schedules, so they can easily become out-of-date.

Depending on the arguments used, :command:`find_package` may use one or both
of the above methods.  By restricting the options to just the basic signature,
both config mode and module mode can be used to satisfy the dependency.
The presence of other options may restrict the call to using only one of the
two methods, potentially reducing the command's ability to find the dependency.
See the :command:`find_package` documentation for full details about this
complex topic.

For both search methods, the user can also set cache variables on the
:manual:`cmake(1)` command line or in the :manual:`ccmake(1)` or
:manual:`cmake-gui(1)` UI tools to influence and override where to find
packages. See the :ref:`User Interaction Guide <Setting Build Variables>`
for more on how to set cache variables.

.. _Libraries providing Config-file packages:

Config-file packages
--------------------

The preferred way for a third party to provide executables, libraries,
headers, and other files for use with CMake is to provide
:ref:`config files <Config File Packages>`.  These are text files shipped
with the package, which define CMake targets, variables, commands, and so on.
The config file is an ordinary CMake script, which is read in by the
:command:`find_package` command.

The config files can usually be found in a directory whose name matches the
pattern ``lib/cmake/<PackageName>``, although they may be in other locations
instead (see :ref:`search procedure`).  The ``<PackageName>`` is usually the
first argument to the :command:`find_package` command, and it may even be the
only argument.  Alternative names can also be specified with the ``NAMES``
option:

.. code-block:: cmake
  :caption: Providing alternative names when finding a package

  find_package(SomeThing
    NAMES
      SameThingOtherName   # Another name for the package
      SomeThing            # Also still look for its canonical name
  )

The config file must be named either ``<PackageName>Config.cmake`` or
``<LowercasePackageName>-config.cmake`` (the former is used for the remainder
of this guide, but both are supported).  This file is the entry point
to the package for CMake.  A separate optional file named
``<PackageName>ConfigVersion.cmake`` or
``<LowercasePackageName>-config-version.cmake`` may also exist in the same
directory.  This file is used by CMake to determine whether the version of
the package satisfies any version constraint included in the call to
:command:`find_package`.  It is optional to specify a version when calling
:command:`find_package`, even if a ``<PackageName>ConfigVersion.cmake``
file is present.

If the ``<PackageName>Config.cmake`` file is found and any version constraint
is satisfied, the :command:`find_package` command considers the package to be
found, and the entire package is assumed to be complete as designed.

There may be additional files providing CMake commands or
:ref:`imported targets` for you to use.  CMake does not enforce any naming
convention for these files.  They are related to the primary
``<PackageName>Config.cmake`` file by use of the CMake :command:`include`
command.  The ``<PackageName>Config.cmake`` file would typically include
these for you, so they won't usually require any additional step other than
the call to :command:`find_package`.

If the location of the package is in a
:ref:`directory known to CMake <search procedure>`, the
:command:`find_package` call should succeed.  The directories known to CMake
are platform-specific.  For example, packages installed on Linux with a
standard system package manager will be found in the ``/usr`` prefix
automatically.  Packages installed in ``Program Files`` on Windows will
similarly be found automatically.

Packages will not be found automatically without help if they are in
locations not known to CMake, such as ``/opt/mylib`` or ``$HOME/dev/prefix``.
This is a normal situation, and CMake provides several ways for users to
specify where to find such libraries.

The :variable:`CMAKE_PREFIX_PATH` variable may be
:ref:`set when invoking CMake <Setting Build Variables>`.
It is treated as a list of base paths in which to search for
:ref:`config files <Config File Packages>`.  A package installed in
``/opt/somepackage`` will typically install config files such as
``/opt/somepackage/lib/cmake/somePackage/SomePackageConfig.cmake``.
In that case, ``/opt/somepackage`` should be added to
:variable:`CMAKE_PREFIX_PATH`.

The environment variable ``CMAKE_PREFIX_PATH`` may also be populated with
prefixes to search for packages.  Like the ``PATH`` environment variable,
this is a list, but it needs to use the platform-specific environment variable
list item separator (``:`` on Unix and ``;`` on Windows).

The :variable:`CMAKE_PREFIX_PATH` variable provides convenience in cases
where multiple prefixes need to be specified, or when multiple packages
are available under the same prefix.  Paths to packages may also be
specified by setting variables matching ``<PackageName>_DIR``, such as
``SomePackage_DIR``.  Note that this is not a prefix, but should be a full
path to a directory containing a config-style package file, such as
``/opt/somepackage/lib/cmake/SomePackage`` in the above example.
See the :command:`find_package` documentation for other CMake variables and
environment variables that can affect the search.

.. _Libraries not Providing Config-file Packages:

Find Module Files
-----------------

Packages which do not provide config files can still be found with the
:command:`find_package` command, if a ``FindSomePackage.cmake`` file is
available.  These Find module files are different to config files in that:

#. Find module files should not be provided by the package itself.
#. The availability of a ``Find<PackageName>.cmake`` file does not indicate
   the availability of the package, or any particular part of the package.
#. CMake does not search the locations specified in the
   :variable:`CMAKE_PREFIX_PATH` variable for ``Find<PackageName>.cmake``
   files.  Instead, CMake searches for such files in the locations given
   by the :variable:`CMAKE_MODULE_PATH` variable.  It is common for users to
   set the :variable:`CMAKE_MODULE_PATH` when running CMake, and it is common
   for CMake projects to append to :variable:`CMAKE_MODULE_PATH` to allow use
   of local Find module files.
#. CMake ships ``Find<PackageName>.cmake`` files for some
   :manual:`third party packages <cmake-modules(7)>`.  These files are a
   maintenance burden for CMake, and it is not unusual for these to fall
   behind the latest releases of the packages they are associated with.
   In general, new Find modules are not added to CMake any more.  Projects
   should encourage the upstream packages to provide a config file where
   possible.  If that is unsuccessful, the project should provide its own
   Find module for the package.

See :ref:`Find Modules` for a detailed discussion of how to write a
Find module file.

.. _Imported Targets from Packages:

Imported Targets
----------------

Both config files and Find module files can define :ref:`Imported targets`.
These will typically have names of the form ``SomePrefix::ThingName``.
Where these are available, the project should prefer to use them instead of
any CMake variables that may also be provided.  Such targets typically carry
usage requirements and apply things like header search paths, compiler
definitions, etc. automatically to other targets that link to them (e.g. using
:command:`target_link_libraries`).  This is both more robust and more
convenient than trying to apply the same things manually using variables.
Check the documentation for the package or Find module to see what imported
targets it defines, if any.

Imported targets should also encapsulate any configuration-specific paths.
This includes the location of binaries (libraries, executables), compiler
flags, and any other configuration-dependent quantities.  Find modules may
be less reliable in providing these details than config files.

A complete example which finds a third party package and uses a library
from it might look like the following:

.. code-block:: cmake

  cmake_minimum_required(VERSION 3.10)
  project(MyExeProject VERSION 1.0.0)

  # Make project-provided Find modules available
  list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

  find_package(SomePackage REQUIRED)
  add_executable(MyExe main.cpp)
  target_link_libraries(MyExe PRIVATE SomePrefix::LibName)

Note that the above call to :command:`find_package` could be resolved by
a config file or a Find module.  It uses only the basic arguments supported
by the :ref:`basic signature`.  A ``FindSomePackage.cmake`` file in the
``${CMAKE_CURRENT_SOURCE_DIR}/cmake`` directory would allow the
:command:`find_package` command to succeed using module mode, for example.
If no such module file is present, the system would be searched for a config
file.


Downloading And Building From Source With ``FetchContent``
==========================================================

Dependencies do not necessarily have to be pre-built in order to use them
with CMake.  They can be built from sources as part of the main project.
The :module:`FetchContent` module provides functionality to download
content (typically sources, but can be anything) and add it to the main
project if the dependency also uses CMake.  The dependency's sources will
be built along with the rest of the project, just as though the sources were
part of the project's own sources.

The general pattern is that the project should first declare all the
dependencies it wants to use, then ask for them to be made available.
The following demonstrates the principle (see :ref:`fetch-content-examples`
for more):

.. code-block:: cmake

  include(FetchContent)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        703bd9caab50b139428cea1aaff9974ebee5742e # release-1.10.0
  )
  FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        de6fe184a9ac1a06895cdd1c9b437f0a0bdf14ad # v2.13.4
  )
  FetchContent_MakeAvailable(googletest Catch2)

Various download methods are supported, including downloading and extracting
archives from a URL (a range of archive formats are supported), and a number
of repository formats including Git, Subversion, and Mercurial.
Custom download, update, and patch commands can also be used to support
arbitrary use cases.

When a dependency is added to the project with :module:`FetchContent`, the
project links to the dependency's targets just like any other target from the
project.  If the dependency provides namespaced targets of the form
``SomePrefix::ThingName``, the project should link to those rather than to
any non-namespaced targets.  See the next section for why this is recommended.

Not all dependencies can be brought into the project this way.  Some
dependencies define targets whose names clash with other targets from the
project or other dependencies.  Concrete executable and library targets
created by :command:`add_executable` and :command:`add_library` are global,
so each one must be unique across the whole build.  If a dependency would
add a clashing target name, it cannot be brought directly into the build
with this method.

``FetchContent`` And ``find_package()`` Integration
===================================================

.. versionadded:: 3.24

Some dependencies support being added by either :command:`find_package` or
:module:`FetchContent`.  Such dependencies must ensure they define the same
namespaced targets in both installed and built-from-source scenarios.
A consuming project then links to those namespaced targets and can handle
both scenarios transparently, as long as the project does not use anything
else that isn't provided by both methods.

The project can indicate it is happy to accept a dependency by either method
using the ``FIND_PACKAGE_ARGS`` option to :command:`FetchContent_Declare`.
This allows :command:`FetchContent_MakeAvailable` to try satisfying the
dependency with a call to :command:`find_package` first, using the arguments
after the ``FIND_PACKAGE_ARGS`` keyword, if any.  If that doesn't find the
dependency, it is built from source as described previously instead.

.. code-block:: cmake

  include(FetchContent)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        703bd9caab50b139428cea1aaff9974ebee5742e # release-1.10.0
    FIND_PACKAGE_ARGS NAMES GTest
  )
  FetchContent_MakeAvailable(googletest)

  add_executable(ThingUnitTest thing_ut.cpp)
  target_link_libraries(ThingUnitTest GTest::gtest_main)

The above example calls
:command:`find_package(googletest NAMES GTest) <find_package>` first.
CMake provides a :module:`FindGTest` module, so if that finds a GTest package
installed somewhere, it will make it available, and the dependency will not be
built from source.  If no GTest package is found, it *will* be built from
source.  In either case, the ``GTest::gtest_main`` target is expected to be
defined, so we link our unit test executable to that target.

High-level control is also available through the
:variable:`FETCHCONTENT_TRY_FIND_PACKAGE_MODE` variable.  This can be set to
``NEVER`` to disable all redirection to :command:`find_package`.  It can be
set to ``ALWAYS`` to try :command:`find_package` even if ``FIND_PACKAGE_ARGS``
was not specified (this should be used with caution).

The project might also decide that a particular dependency must be built from
source.  This might be needed if a patched or unreleased version of the
dependency is required, or to satisfy some policy that requires all
dependencies to be built from source.  The project can enforce this by adding
the ``OVERRIDE_FIND_PACKAGE`` keyword to :command:`FetchContent_Declare`.
A call to :command:`find_package` for that dependency will then be redirected
to :command:`FetchContent_MakeAvailable` instead.

.. code-block:: cmake

  include(FetchContent)
  FetchContent_Declare(
    Catch2
    URL https://intranet.mycomp.com/vendored/Catch2_2.13.4_patched.tgz
    URL_HASH MD5=abc123...
    OVERRIDE_FIND_PACKAGE
  )

  # The following is automatically redirected to FetchContent_MakeAvailable(Catch2)
  find_package(Catch2)

For more advanced use cases, see the
:variable:`CMAKE_FIND_PACKAGE_REDIRECTS_DIR` variable.

.. _dependency_providers_overview:

Dependency Providers
====================

.. versionadded:: 3.24

The preceding section discussed techniques that projects can use to specify
their dependencies.  Ideally, the project shouldn't really care where a
dependency comes from, as long as it provides the things it expects (often
just some imported targets).  The project says what it needs and may also
specify where to get it from, in the absence of any other details, so that it
can still be built out-of-the-box.

The developer, on the other hand, may be much more interested in controlling
*how* a dependency is provided to the project.  You might want to use a
particular version of a package that you built yourself.  You might want
to use a third party package manager.  You might want to redirect some
requests to a different URL on a system you control for security or
performance reasons.  CMake supports these sort of scenarios through
:ref:`dependency_providers`.

A dependency provider can be set to intercept :command:`find_package` and
:command:`FetchContent_MakeAvailable` calls.  The provider is given an
opportunity to satisfy such requests before falling back to the built-in
implementation if the provider doesn't fulfill it.

Only one dependency provider can be set, and it can only be set at a very
specific point early in the CMake run.
The :variable:`CMAKE_PROJECT_TOP_LEVEL_INCLUDES` variable lists CMake files
that will be read while processing the first :command:`project()` call (and
only that call).  This is the only time a dependency provider may be set.
At most, one single provider is expected to be used throughout the whole
project.

For some scenarios, the user wouldn't need to know the details of how the
dependency provider is set.  A third party may provide a file that can be
added to :variable:`CMAKE_PROJECT_TOP_LEVEL_INCLUDES`, which will set up
the dependency provider on the user's behalf.  This is the recommended
approach for package managers.  The developer can use such a file like so::

  cmake -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=/path/to/package_manager/setup.cmake ...

For details on how to implement your own custom dependency provider, see the
:command:`cmake_language(SET_DEPENDENCY_PROVIDER)` command.
