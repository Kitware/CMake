macro
-----

Start recording a macro for later invocation as a command.

::

  macro(<name> [arg1 [arg2 [arg3 ...]]])
    COMMAND1(ARGS ...)
    COMMAND2(ARGS ...)
    ...
  endmacro(<name>)

Define a macro named <name> that takes arguments named arg1 arg2 arg3
(...).  Commands listed after macro, but before the matching endmacro,
are not invoked until the macro is invoked.  When it is invoked, the
commands recorded in the macro are first modified by replacing formal
parameters (${arg1}) with the arguments passed, and then invoked as
normal commands.  In addition to referencing the formal parameters you
can reference the values ${ARGC} which will be set to the number of
arguments passed into the function as well as ${ARGV0} ${ARGV1}
${ARGV2} ...  which will have the actual values of the arguments
passed in.  This facilitates creating macros with optional arguments.
Additionally ${ARGV} holds the list of all arguments given to the
macro and ${ARGN} holds the list of arguments past the last expected
argument.  Note that the parameters to a macro and values such as ARGN
are not variables in the usual CMake sense.  They are string
replacements much like the C preprocessor would do with a macro.  If
you want true CMake variables and/or better CMake scope control you
should look at the function command.

See the cmake_policy() command documentation for the behavior of
policies inside macros.
