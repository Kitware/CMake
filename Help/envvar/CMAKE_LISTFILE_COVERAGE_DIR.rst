CMAKE_LISTFILE_COVERAGE_DIR
---------------------------

.. versionadded:: 4.6

.. include:: include/ENV_VAR.rst

Record coverage of CMake language code in lcov tracefiles.

When this environment variable is set to a directory, each :program:`cmake`,
:program:`ctest` and :program:`cpack` process that runs CMake language code
records which lines of each listfile ran, how many times each function and
macro was called, and which way each decision went.
At exit the process writes this data into the directory.  This variable alone
turns coverage on.  There is no command-line option for it.

The value must be an absolute path.  If the directory does not exist,
CMake creates it.  If the variable is not set, or is set to an empty value,
coverage is off.

Child processes inherit the variable, so a :program:`cmake` process that a
project starts with :command:`execute_process` also records coverage.  On
every platform, this form sets the variable for one command:

.. code-block:: console

  $ cmake -E env CMAKE_LISTFILE_COVERAGE_DIR=/path/to/dir cmake -S src -B build

Modes that Record Coverage
^^^^^^^^^^^^^^^^^^^^^^^^^^

These modes record coverage:

* Configure and generate a project.  This includes projects that
  :command:`try_compile` and :command:`try_run` configure, which record into
  the same file as the parent project.
* Script mode, :option:`cmake -P`.
* Install scripts, :option:`cmake --install`.
* A re-run of configure that the build system starts.
* :manual:`ctest(1)`, for the CMake language code that it runs itself, such
  as ``CTestTestfile.cmake`` files and :ref:`Dashboard Client` scripts.
  To record coverage for each test instead, use the ``CMAKE`` value of the
  CTest :ref:`CoverageTool <ctest-CoverageTool>` setting.
* :manual:`cpack(1)`, for the CPack configuration file, the CPack generator
  modules and the install scripts that it runs to make a package.

These modes do not record coverage and do not check the directory:

* :manual:`ccmake(1)` and :manual:`cmake-gui(1)`.
* :option:`cmake --build` and the :option:`cmake -E` commands.  This
  includes ``cmake -E cmake_depends``, which reads files that CMake
  generates.
* :option:`cmake --help`, :option:`cmake --version`, :option:`ctest --help`,
  :option:`ctest --version`, :option:`cpack --help` and
  :option:`cpack --version`.
* ``cmake --system-information``.
* ``cmake --check-build-system``, which the build system runs to check
  whether it must re-run configure.  If the check decides to re-run
  configure, coverage starts at that point and the re-run records as usual.
  The ``PreLoad.cmake`` file that CMake reads before the check is not
  recorded in that run.
* The compile and link launchers that :module:`CTestUseLaunchers` enables,
  which read a file that CMake generates, once for each compile.

Some other modes that run no CMake language code, such as ``cmake -N`` and
:option:`cmake --list-presets`, check the directory at startup, but write no
file.

Errors
^^^^^^

Before it runs any CMake language code, CMake makes sure that it can write
files in the directory.  If the value is a relative path, or if CMake cannot
create the directory or write there, CMake reports an error and stops.  A
failure to write the file at exit does not change the exit code.

What Gets Recorded
^^^^^^^^^^^^^^^^^^

Each file that CMake reads as a listfile gets a record, with the absolute
path that CMake used to read it.  This includes files under
:variable:`CMAKE_ROOT` and files that CMake generates in the build tree.
Records can name generated files that CMake deletes later, such as the
projects that :command:`try_compile` configures, so report tools may need
to filter them out.  The CTest ``CMAKE``
:ref:`coverage tool <ctest-CoverageTool>` leaves out the files under the
build tree when it merges the files of a test, except for an in-source
build.  Files that CMake never reads do not appear.  A file with no line
that can count and no function or macro, such as a file with only comments,
gets no record, and a process that has no record writes no file.

Lines
"""""

* A command counts on the line where it starts, once each time it runs, even
  if its arguments span several lines.
* Opening commands such as :command:`if`, :command:`foreach`,
  :command:`while`, :command:`block`, :command:`function` and
  :command:`macro` count like any other command.  A :command:`while`
  command counts once each time the loop starts, not once for each
  evaluation of its condition.
* An :command:`elseif` counts each time CMake evaluates its condition.
* :command:`else` and the commands that close a block, such as
  :command:`endif` and :command:`endfunction`, do not count.  Comments and
  blank lines do not count.
* A command in the body of a function or macro counts each time the body
  runs, on its line in the file that defines the function or macro.
* A deferred call made with :command:`cmake_language(DEFER)` counts on its
  own line when it runs.
* A command that CMake skips because of an earlier fatal error does not
  count.
* A command that another command runs on the same line does not add a hit.
  Examples are the command that :command:`cmake_language(CALL)` runs, and
  the commands that :command:`list` operations call.
* Code that :command:`cmake_language(EVAL CODE) <cmake_language(EVAL)>` runs
  is left out, because it has no file.  The :command:`cmake_language` line
  itself counts.
* Each line that can count appears in the record, with zero hits if it
  never ran.

Functions and Macros
""""""""""""""""""""

* Each :command:`function` and :command:`macro` definition gets one
  function record, with the name as written in the source and the line of
  the definition.  Its count is the number of calls.
* If one file defines the same name more than once, each definition gets
  its own record, named ``<name>@<line>``, for example ``foo@120`` and
  ``foo@135``.  A name that one file defines once keeps its name.
* A name is percent-encoded as in URLs: ``%`` is written as ``%25``, a
  comma as ``%2C``, a line break as ``%0A`` or ``%0D``, and ``@`` as
  ``%40``.  Other characters are written as they are.  For example, a
  function named ``a,b`` is written as ``a%2Cb``.
* When CMake reads a tracefile back, it removes the ``@<line>`` suffix and
  decodes the name, so every name reads back as written.

Branches
""""""""

* Each :command:`if`, :command:`elseif`, :command:`while` and
  :command:`foreach` line gets one pair of branches.  Branch 0 is true or
  enter.  Branch 1 is false or leave.
* An :command:`elseif` that CMake does not evaluate adds nothing.
  :command:`else` has no record.
* A :command:`while` counts enter each time its condition is true, and leave
  when its condition is false.
* A :command:`foreach` counts enter for each item.  It counts leave when no
  item is left, including for a loop with zero items.
* A loop that ends any other way, such as with :command:`break` or
  :command:`return`, does not count leave.
* A condition that fails with a fatal error counts as neither branch.  Its
  line still counts as run, so lcov 2.x reports inconsistent data for it.
  The ``lcov`` option ``--ignore-errors inconsistent`` accepts it.
* A decision in the body of a function or macro counts in the file that
  defines it.
* Only the four commands above record branches.  A choice that another
  command or a generator expression makes has no branch record.

Output Files
^^^^^^^^^^^^

Each process that records coverage writes one lcov tracefile, named
``<pid>-<token>.info``, where ``<pid>`` is the process ID and ``<token>`` is a
random value.  Each source file starts with an ``SF`` record.  The file uses
these other records:

* ``DA``, ``LF`` and ``LH`` for lines.
* ``FN``, ``FNDA``, ``FNF`` and ``FNH`` for functions and macros.
* ``BRDA``, ``BRF`` and ``BRH`` for branches.  The
  ``genhtml --branch-coverage`` command shows them.

CMake never deletes or merges files in the directory.  Clear the directory
before a new run.
