cmake-gui(1)
************

::

  cmake-gui - CMake GUI.

::

  cmake-gui [options]
  cmake-gui [options] <path-to-source>
  cmake-gui [options] <path-to-existing-build>

The "cmake-gui" executable is the CMake GUI.  Project configuration
settings may be specified interactively.  Brief instructions are
provided at the bottom of the window when the program is running.

CMake is a cross-platform build system generator.  Projects specify
their build process with platform-independent CMake listfiles included
in each directory of a source tree with the name CMakeLists.txt.
Users build a project by using CMake to generate a build system for a
native tool on their platform.


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
