exec_program
------------

.. versionchanged:: 3.28
  This command is available only if policy :policy:`CMP0153` is not set to ``NEW``.
  Port projects to the :command:`execute_process` command.

.. deprecated:: 3.0

  Use the :command:`execute_process` command instead.

Run an executable program during the processing of the CMakeList.txt
file.

.. code-block:: cmake

  exec_program(Executable [directory in which to run]
               [ARGS <arguments to executable>]
               [OUTPUT_VARIABLE <var>]
               [RETURN_VALUE <var>])

The executable is run in the optionally specified directory.  The
executable can include arguments if it is double quoted, but it is
better to use the optional ``ARGS`` argument to specify arguments to the
program.  This is because cmake will then be able to escape spaces in
the executable path.  An optional argument ``OUTPUT_VARIABLE`` specifies a
variable in which to store the output.  To capture the return value of
the execution, provide a ``RETURN_VALUE``.  If ``OUTPUT_VARIABLE`` is
specified, then no output will go to the stdout/stderr of the console
running cmake.
