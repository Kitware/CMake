cmake(1)
********

Synopsis
========

::

  cmake [options] <path-to-source>
  cmake [options] <path-to-existing-build>

Description
===========

The "cmake" executable is the CMake command-line interface.  It may be
used to configure projects in scripts.  Project configuration settings
may be specified on the command line with the -D option.  The -i
option will cause cmake to interactively prompt for such settings.

CMake is a cross-platform build system generator.  Projects specify
their build process with platform-independent CMake listfiles included
in each directory of a source tree with the name CMakeLists.txt.
Users build a project by using CMake to generate a build system for a
native tool on their platform.

.. include:: OPTIONS_BUILD.txt

* ``-E``: CMake command mode.

  For true platform independence, CMake provides a list of commands
  that can be used on all systems.  Run with -E help for the usage
  information.  Commands available are: chdir, compare_files, copy,
  copy_directory, copy_if_different, echo, echo_append, environment,
  make_directory, md5sum, remove, remove_directory, rename, tar, time,
  touch, touch_nocreate.  In addition, some platform specific commands
  are available.  On Windows: comspec, delete_regv, write_regv.  On
  UNIX: create_symlink.

* ``-i``: Run in wizard mode.

  Wizard mode runs cmake interactively without a GUI.  The user is
  prompted to answer questions about the project configuration.  The
  answers are used to set cmake cache values.

* ``-L[A][H]``: List non-advanced cached variables.

  List cache variables will run CMake and list all the variables from
  the CMake cache that are not marked as INTERNAL or ADVANCED.  This
  will effectively display current CMake settings, which can then be
  changed with -D option.  Changing some of the variables may result
  in more variables being created.  If A is specified, then it will
  display also advanced variables.  If H is specified, it will also
  display help for each variable.

* ``--build <dir>``: Build a CMake-generated project binary tree.

  This abstracts a native build tool's command-line interface with the
  following options:

  ::

    <dir>          = Project binary directory to be built.
    --target <tgt> = Build <tgt> instead of default targets.
    --config <cfg> = For multi-configuration tools, choose <cfg>.
    --clean-first  = Build target 'clean' first, then build.
                     (To clean only, use --target 'clean'.)
    --use-stderr   = Don't merge stdout/stderr output and pass the
                     original stdout/stderr handles to the native
                     tool so it can use the capabilities of the
                     calling terminal (e.g. colored output).
    --             = Pass remaining options to the native tool.

  Run cmake --build with no options for quick help.

* ``-N``: View mode only.

  Only load the cache.  Do not actually run configure and generate
  steps.

* ``-P <file>``: Process script mode.

  Process the given cmake file as a script written in the CMake
  language.  No configure or generate step is performed and the cache
  is not modified.  If variables are defined using -D, this must be
  done before the -P argument.

* ``--find-package``: Run in pkg-config like mode.

  Search a package using find_package() and print the resulting flags
  to stdout.  This can be used to use cmake instead of pkg-config to
  find installed libraries in plain Makefile-based projects or in
  autoconf-based projects (via share/aclocal/cmake.m4).

* ``--graphviz=[file]``: Generate graphviz of dependencies, see CMakeGraphVizOptions.cmake for more.

  Generate a graphviz input file that will contain all the library and
  executable dependencies in the project.  See the documentation for
  CMakeGraphVizOptions.cmake for more details.

* ``--system-information [file]``: Dump information about this system.

  Dump a wide range of information about the current system.  If run
  from the top of a binary tree for a CMake project it will dump
  additional information such as the cache, log files etc.

* ``--debug-trycompile``: Do not delete the try_compile build tree. Only useful on one try_compile at a time.

  Do not delete the files and directories created for try_compile
  calls.  This is useful in debugging failed try_compiles.  It may
  however change the results of the try-compiles as old junk from a
  previous try-compile may cause a different test to either pass or
  fail incorrectly.  This option is best used for one try-compile at a
  time, and only when debugging.

* ``--debug-output``: Put cmake in a debug mode.

  Print extra stuff during the cmake run like stack traces with
  message(send_error ) calls.

* ``--trace``: Put cmake in trace mode.

  Print a trace of all calls made and from where with
  message(send_error ) calls.

* ``--warn-uninitialized``: Warn about uninitialized values.

  Print a warning when an uninitialized variable is used.

* ``--warn-unused-vars``: Warn about unused variables.

  Find variables that are declared or set, but not used.

* ``--no-warn-unused-cli``: Don't warn about command line options.

  Don't find variables that are declared on the command line, but not
  used.

* ``--check-system-vars``: Find problems with variable usage in system files.

  Normally, unused and uninitialized variables are searched for only
  in CMAKE_SOURCE_DIR and CMAKE_BINARY_DIR.  This flag tells CMake to
  warn about other files as well.

* ``--help-command cmd [file]``: Print help for a single command and exit.

  Full documentation specific to the given command is displayed.  If a
  file is specified, the documentation is written into and the output
  format is determined depending on the filename suffix.  Supported
  are man page, HTML, DocBook and plain text.

* ``--help-command-list [file]``: List available listfile commands and exit.

  The list contains all commands for which help may be obtained by
  using the --help-command argument followed by a command name.  If a
  file is specified, the documentation is written into and the output
  format is determined depending on the filename suffix.  Supported
  are man page, HTML, DocBook and plain text.

* ``--help-commands [file]``: Print help for all commands and exit.

  Full documentation specific for all current commands is displayed.If
  a file is specified, the documentation is written into and the
  output format is determined depending on the filename suffix.
  Supported are man page, HTML, DocBook and plain text.

* ``--help-compatcommands [file]``: Print help for compatibility commands.

  Full documentation specific for all compatibility commands is
  displayed.If a file is specified, the documentation is written into
  and the output format is determined depending on the filename
  suffix.  Supported are man page, HTML, DocBook and plain text.

* ``--help-module module [file]``: Print help for a single module and exit.

  Full documentation specific to the given module is displayed.If a
  file is specified, the documentation is written into and the output
  format is determined depending on the filename suffix.  Supported
  are man page, HTML, DocBook and plain text.

* ``--help-module-list [file]``: List available modules and exit.

  The list contains all modules for which help may be obtained by
  using the --help-module argument followed by a module name.  If a
  file is specified, the documentation is written into and the output
  format is determined depending on the filename suffix.  Supported
  are man page, HTML, DocBook and plain text.

* ``--help-modules [file]``: Print help for all modules and exit.

  Full documentation for all modules is displayed.  If a file is
  specified, the documentation is written into and the output format
  is determined depending on the filename suffix.  Supported are man
  page, HTML, DocBook and plain text.

* ``--help-custom-modules [file]``: Print help for all custom modules and exit.

  Full documentation for all custom modules is displayed.  If a file
  is specified, the documentation is written into and the output
  format is determined depending on the filename suffix.  Supported
  are man page, HTML, DocBook and plain text.

* ``--help-policy cmp [file]``: Print help for a single policy and exit.

  Full documentation specific to the given policy is displayed.If a
  file is specified, the documentation is written into and the output
  format is determined depending on the filename suffix.  Supported
  are man page, HTML, DocBook and plain text.

* ``--help-policy-list [file]``: List available policies and exit.

  The list contains all policies for which help may be obtained by
  using the --help-policy argument followed by a policy name.  If a
  file is specified, the documentation is written into and the output
  format is determined depending on the filename suffix.  Supported
  are man page, HTML, DocBook and plain text.

* ``--help-policies [file]``: Print help for all policies and exit.

  Full documentation for all policies is displayed.If a file is
  specified, the documentation is written into and the output format
  is determined depending on the filename suffix.  Supported are man
  page, HTML, DocBook and plain text.

* ``--help-property prop [file]``: Print help for a single property and exit.

  Full documentation specific to the given property is displayed.If a
  file is specified, the documentation is written into and the output
  format is determined depending on the filename suffix.  Supported
  are man page, HTML, DocBook and plain text.

* ``--help-property-list [file]``: List available properties and exit.

  The list contains all properties for which help may be obtained by
  using the --help-property argument followed by a property name.  If
  a file is specified, the help is written into it.If a file is
  specified, the documentation is written into and the output format
  is determined depending on the filename suffix.  Supported are man
  page, HTML, DocBook and plain text.

* ``--help-properties [file]``: Print help for all properties and exit.

  Full documentation for all properties is displayed.If a file is
  specified, the documentation is written into and the output format
  is determined depending on the filename suffix.  Supported are man
  page, HTML, DocBook and plain text.

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

See Also
========

.. include:: LINKS.txt
