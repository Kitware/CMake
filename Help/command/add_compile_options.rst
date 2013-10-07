add_compile_options
-------------------

Adds options to the compilation of source files.

::

  add_compile_options(<option> ...)

Adds options to the compiler command line for sources in the current
directory and below.  This command can be used to add any options, but
alternative commands exist to add preprocessor definitions or include
directories.  See documentation of the directory and target
COMPILE_OPTIONS properties for details.  Arguments to
add_compile_options may use "generator expressions" with the syntax
"$<...>".  See the :manual:`cmake-generator-expressions(7)` manual for
available expressions.
