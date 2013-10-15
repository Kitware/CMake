execute_process
---------------

Execute one or more child processes.

::

  execute_process(COMMAND <cmd1> [args1...]]
                  [COMMAND <cmd2> [args2...] [...]]
                  [WORKING_DIRECTORY <directory>]
                  [TIMEOUT <seconds>]
                  [RESULT_VARIABLE <variable>]
                  [OUTPUT_VARIABLE <variable>]
                  [ERROR_VARIABLE <variable>]
                  [INPUT_FILE <file>]
                  [OUTPUT_FILE <file>]
                  [ERROR_FILE <file>]
                  [OUTPUT_QUIET]
                  [ERROR_QUIET]
                  [OUTPUT_STRIP_TRAILING_WHITESPACE]
                  [ERROR_STRIP_TRAILING_WHITESPACE])

Runs the given sequence of one or more commands with the standard
output of each process piped to the standard input of the next.  A
single standard error pipe is used for all processes.  If
WORKING_DIRECTORY is given the named directory will be set as the
current working directory of the child processes.  If TIMEOUT is given
the child processes will be terminated if they do not finish in the
specified number of seconds (fractions are allowed).  If
RESULT_VARIABLE is given the variable will be set to contain the
result of running the processes.  This will be an integer return code
from the last child or a string describing an error condition.  If
OUTPUT_VARIABLE or ERROR_VARIABLE are given the variable named will be
set with the contents of the standard output and standard error pipes
respectively.  If the same variable is named for both pipes their
output will be merged in the order produced.  If INPUT_FILE,
OUTPUT_FILE, or ERROR_FILE is given the file named will be attached to
the standard input of the first process, standard output of the last
process, or standard error of all processes respectively.  If
OUTPUT_QUIET or ERROR_QUIET is given then the standard output or
standard error results will be quietly ignored.  If more than one
OUTPUT_* or ERROR_* option is given for the same pipe the precedence
is not specified.  If no OUTPUT_* or ERROR_* options are given the
output will be shared with the corresponding pipes of the CMake
process itself.

The execute_process command is a newer more powerful version of
exec_program, but the old command has been kept for compatibility.
