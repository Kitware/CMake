cpack(1)
********

::

  cpack - Packaging driver provided by CMake.

::

  cpack -G <generator> [options]

The "cpack" executable is the CMake packaging program.
CMake-generated build trees created for projects that use the
INSTALL_* commands have packaging support.  This program will generate
the package.

CMake is a cross-platform build system generator.  Projects specify
their build process with platform-independent CMake listfiles included
in each directory of a source tree with the name CMakeLists.txt.
Users build a project by using CMake to generate a build system for a
native tool on their platform.


* ``-G <generator>``: Use the specified generator to generate package.

  CPack may support multiple native packaging systems on certain
  platforms.  A generator is responsible for generating input files
  for particular system and invoking that systems.  Possible generator
  names are specified in the Generators section.

* ``-C <Configuration>``: Specify the project configuration

  This option specifies the configuration that the project was build
  with, for example 'Debug', 'Release'.

* ``-D <var>=<value>``: Set a CPack variable.

  Set a variable that can be used by the generator.

* ``--config <config file>``: Specify the config file.

  Specify the config file to use to create the package.  By default
  CPackConfig.cmake in the current directory will be used.

* ``--verbose,-V``: enable verbose output

  Run cpack with verbose output.

* ``--debug``: enable debug output (for CPack developers)

  Run cpack with debug output (for CPack developers).

* ``-P <package name>``: override/define CPACK_PACKAGE_NAME

  If the package name is not specified on cpack commmand line
  thenCPack.cmake defines it as CMAKE_PROJECT_NAME

* ``-R <package version>``: override/define CPACK_PACKAGE_VERSION

  If version is not specified on cpack command line thenCPack.cmake
  defines it from CPACK_PACKAGE_VERSION_[MAJOR|MINOR|PATCH]look into
  CPack.cmake for detail

* ``-B <package directory>``: override/define CPACK_PACKAGE_DIRECTORY

  The directory where CPack will be doing its packaging work.The
  resulting package will be found there.  Inside this directoryCPack
  creates '_CPack_Packages' sub-directory which is theCPack temporary
  directory.

* ``--vendor <vendor name>``: override/define CPACK_PACKAGE_VENDOR

  If vendor is not specified on cpack command line (or inside
  CMakeLists.txt) thenCPack.cmake defines it with a default value

* ``--help-command cmd [file]``: Print help for a single command and exit.

  Full documentation specific to the given command is displayed.  If a
  file is specified, the documentation is written into and the output
  format is determined depending on the filename suffix.  Supported
  are man page, HTML, DocBook and plain text.

* ``--help-command-list [file]``: List available commands and exit.

  The list contains all commands for which help may be obtained by
  using the --help-command argument followed by a command name.  If a
  file is specified, the documentation is written into and the output
  format is determined depending on the filename suffix.  Supported
  are man page, HTML, DocBook and plain text.

* ``--help-commands [file]``: Print help for all commands and exit.

  Full documentation specific for all current command is displayed.If
  a file is specified, the documentation is written into and the
  output format is determined depending on the filename suffix.
  Supported are man page, HTML, DocBook and plain text.

* ``--help-variable var [file]``: Print help for a single variable and exit.

  Full documentation specific to the given variable is displayed.If a
  file is specified, the documentation is written into and the output
  format is determined depending on the filename suffix.  Supported
  are man page, HTML, DocBook and plain text.

* ``--help-variable-list [file]``: List documented variables and exit.

  The list contains all variables for which help may be obtained by
  using the --help-variable argument followed by a variable name.  If
  a file is specified, the help is written into it.If a file is
  specified, the documentation is written into and the output format
  is determined depending on the filename suffix.  Supported are man
  page, HTML, DocBook and plain text.

* ``--help-variables [file]``: Print help for all variables and exit.

  Full documentation for all variables is displayed.If a file is
  specified, the documentation is written into and the output format
  is determined depending on the filename suffix.  Supported are man
  page, HTML, DocBook and plain text.

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
