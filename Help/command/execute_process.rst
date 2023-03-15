execute_process
---------------

Execute one or more child processes.

.. code-block:: cmake

  execute_process(COMMAND <cmd1> [<arguments>]
                  [COMMAND <cmd2> [<arguments>]]...
                  [WORKING_DIRECTORY <directory>]
                  [TIMEOUT <seconds>]
                  [RESULT_VARIABLE <variable>]
                  [RESULTS_VARIABLE <variable>]
                  [OUTPUT_VARIABLE <variable>]
                  [ERROR_VARIABLE <variable>]
                  [INPUT_FILE <file>]
                  [OUTPUT_FILE <file>]
                  [ERROR_FILE <file>]
                  [OUTPUT_QUIET]
                  [ERROR_QUIET]
                  [COMMAND_ECHO <where>]
                  [OUTPUT_STRIP_TRAILING_WHITESPACE]
                  [ERROR_STRIP_TRAILING_WHITESPACE]
                  [ENCODING <name>]
                  [ECHO_OUTPUT_VARIABLE]
                  [ECHO_ERROR_VARIABLE]
                  [COMMAND_ERROR_IS_FATAL <ANY|LAST>])

Runs the given sequence of one or more commands.

Commands are executed concurrently as a pipeline, with the standard
output of each process piped to the standard input of the next.
A single standard error pipe is used for all processes.

``execute_process`` runs commands while CMake is configuring the project,
prior to build system generation.  Use the :command:`add_custom_target` and
:command:`add_custom_command` commands to create custom commands that run
at build time.

Options:

``COMMAND``
 A child process command line.

 CMake executes the child process using operating system APIs directly:

 * On POSIX platforms, the command line is passed to the
   child process in an ``argv[]`` style array.

 * On Windows platforms, the command line is encoded as a string such
   that child processes using ``CommandLineToArgvW`` will decode the
   original arguments.

 No intermediate shell is used, so shell operators such as ``>``
 are treated as normal arguments.
 (Use the ``INPUT_*``, ``OUTPUT_*``, and ``ERROR_*`` options to
 redirect stdin, stdout, and stderr.)

 For **sequential execution** of multiple commands use multiple
 ``execute_process`` calls each with a single ``COMMAND`` argument.

``WORKING_DIRECTORY``
 The named directory will be set as the current working directory of
 the child processes.

``TIMEOUT``
 After the specified number of seconds (fractions allowed), all unfinished
 child processes will be terminated, and the ``RESULT_VARIABLE`` will be
 set to a string mentioning the "timeout".

``RESULT_VARIABLE``
 The variable will be set to contain the result of last child process.
 This will be an integer return code from the last child or a string
 describing an error condition.

``RESULTS_VARIABLE <variable>``
 .. versionadded:: 3.10

 The variable will be set to contain the result of all processes as a
 :ref:`semicolon-separated list <CMake Language Lists>`, in order of the
 given ``COMMAND`` arguments.  Each entry will be an integer return code
 from the corresponding child or a string describing an error condition.

``INPUT_FILE <file>``
 ``<file>`` is attached to the standard input pipe of the *first* ``COMMAND``
 process.

``OUTPUT_FILE <file>``
 ``<file>`` is attached to the standard output pipe of the *last* ``COMMAND``
 process.

``ERROR_FILE <file>``
 ``<file>`` is attached to the standard error pipe of *all* ``COMMAND``
 processes.

.. versionadded:: 3.3
  If the same ``<file>`` is named for both ``OUTPUT_FILE`` and ``ERROR_FILE``
  then it will be used for both standard output and standard error pipes.

``OUTPUT_QUIET``, ``ERROR_QUIET``
 The standard output on ``OUTPUT_VARIABLE`` or standard error on
 ``ERROR_VARIABLE`` are not connected (no variable content).
 The  ``*_FILE`` and ``ECHO_*_VARIABLE`` options are not affected.

``OUTPUT_VARIABLE``, ``ERROR_VARIABLE``
 The variable named will be set with the contents of the standard output
 and standard error pipes, respectively.  If the same variable is named
 for both pipes their output will be merged in the order produced.

``ECHO_OUTPUT_VARIABLE``, ``ECHO_ERROR_VARIABLE``
  .. versionadded:: 3.18

  The standard output or standard error will not be exclusively redirected to
  the specified variables.

  The output will be duplicated into the specified variables and also onto
  standard output or standard error analogous to the ``tee`` Unix command.

.. note::
  If more than one ``OUTPUT_*`` or ``ERROR_*`` option is given for the
  same pipe the precedence is *not specified*.
  If no ``OUTPUT_*`` or ``ERROR_*`` options are given the output will
  be shared with the corresponding pipes of the CMake process itself.

``COMMAND_ECHO <where>``
 .. versionadded:: 3.15

 The command being run will be echo'ed to ``<where>`` with ``<where>``
 being set to one of ``STDERR``, ``STDOUT`` or ``NONE``.
 See the :variable:`CMAKE_EXECUTE_PROCESS_COMMAND_ECHO` variable for a way
 to control the default behavior when this option is not present.

``ENCODING <name>``
 .. versionadded:: 3.8

 On Windows, the encoding that is used to decode output from the process.
 Ignored on other platforms.
 Valid encoding names are:

 ``NONE``
   Perform no decoding.  This assumes that the process output is encoded
   in the same way as CMake's internal encoding (UTF-8).
   This is the default.
 ``AUTO``
   Use the current active console's codepage or if that isn't
   available then use ANSI.
 ``ANSI``
   Use the ANSI codepage.
 ``OEM``
   Use the original equipment manufacturer (OEM) code page.
 ``UTF8`` or ``UTF-8``
   Use the UTF-8 codepage.

   .. versionadded:: 3.11
     Accept ``UTF-8`` spelling for consistency with the
     `UTF-8 RFC <https://www.ietf.org/rfc/rfc3629>`_ naming convention.

``COMMAND_ERROR_IS_FATAL <ANY|LAST>``
  .. versionadded:: 3.19

  The option following ``COMMAND_ERROR_IS_FATAL`` determines the behavior when
  an error is encountered:

    ``ANY``
    If any of the commands in the list of commands fail, the
    ``execute_process()`` command halts with an error.

    ``LAST``
    If the last command in the list of commands fails, the
    ``execute_process()`` command halts with an error.  Commands earlier in the
    list will not cause a fatal error.
