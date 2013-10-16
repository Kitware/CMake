ccmake(1)
*********

::

  ccmake - Curses Interface for CMake.

::

  ccmake <path-to-source>
  ccmake <path-to-existing-build>

The "ccmake" executable is the CMake curses interface.  Project
configuration settings may be specified interactively through this
GUI.  Brief instructions are provided at the bottom of the terminal
when the program is running.

CMake is a cross-platform build system generator.  Projects specify
their build process with platform-independent CMake listfiles included
in each directory of a source tree with the name CMakeLists.txt.
Users build a project by using CMake to generate a build system for a
native tool on their platform.


* ``-C <initial-cache>``: Pre-load a script to populate the cache.

  When cmake is first run in an empty build tree, it creates a
  CMakeCache.txt file and populates it with customizable settings for
  the project.  This option may be used to specify a file from which
  to load cache entries before the first pass through the project's
  cmake listfiles.  The loaded entries take priority over the
  project's default values.  The given file should be a CMake script
  containing SET commands that use the CACHE option, not a
  cache-format file.

* ``-D <var>:<type>=<value>``: Create a cmake cache entry.

  When cmake is first run in an empty build tree, it creates a
  CMakeCache.txt file and populates it with customizable settings for
  the project.  This option may be used to specify a setting that
  takes priority over the project's default value.  The option may be
  repeated for as many cache entries as desired.

* ``-U <globbing_expr>``: Remove matching entries from CMake cache.

  This option may be used to remove one or more variables from the
  CMakeCache.txt file, globbing expressions using * and ? are
  supported.  The option may be repeated for as many cache entries as
  desired.

  Use with care, you can make your CMakeCache.txt non-working.

* ``-G <generator-name>``: Specify a build system generator.

  CMake may support multiple native build systems on certain
  platforms.  A generator is responsible for generating a particular
  build system.  Possible generator names are specified in the
  Generators section.

* ``-T <toolset-name>``: Specify toolset name if supported by generator.

  Some CMake generators support a toolset name to be given to the
  native build system to choose a compiler.  This is supported only on
  specific generators:

  ::

    Visual Studio >= 10
    Xcode >= 3.0

  See native build system documentation for allowed toolset names.

* ``-Wno-dev``: Suppress developer warnings.

  Suppress warnings that are meant for the author of the
  CMakeLists.txt files.

* ``-Wdev``: Enable developer warnings.

  Enable warnings that are meant for the author of the CMakeLists.txt
  files.

* ``--copyright [file]``: Print the CMake copyright and exit.

  If a file is specified, the copyright is written into it.

* ``--help,-help,-usage,-h,-H,/?``: Print usage information and exit.

  Usage describes the basic command line interface and its options.

* ``--help-full [file]``: Print full help and exit.

  Full help displays most of the documentation provided by the UNIX
  man page.  It is provided for use on non-UNIX platforms, but is also
  convenient if the man page is not installed.  If a file is
  specified, the help is written into it.

* ``--help-html [file]``: Print full help in HTML format.

  This option is used by CMake authors to help produce web pages.  If
  a file is specified, the help is written into it.

* ``--help-man [file]``: Print full help as a UNIX man page and exit.

  This option is used by the cmake build to generate the UNIX man
  page.  If a file is specified, the help is written into it.

* ``--version,-version,/V [file]``: Show program name/version banner and exit.

  If a file is specified, the version is written into it.

The following resources are available to get help using CMake:

* ``Home Page``: http://www.cmake.org

  The primary starting point for learning about CMake.

* ``Frequently Asked Questions``: http://www.cmake.org/Wiki/CMake_FAQ

  A Wiki is provided containing answers to frequently asked questions.

* ``Online Documentation``: http://www.cmake.org/HTML/Documentation.html

  Links to available documentation may be found on this web page.

* ``Mailing List``: http://www.cmake.org/HTML/MailingLists.html

  For help and discussion about using cmake, a mailing list is
  provided at cmake@cmake.org.  The list is member-post-only but one
  may sign up on the CMake web page.  Please first read the full
  documentation at http://www.cmake.org before posting questions to
  the list.
