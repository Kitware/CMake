add_compile_options
-------------------

Adds options to the compilation of source files.

::

  add_compile_options(<option> ...)

Adds options to the compiler command line for sources in the current
directory and below.  This command can be used to add any options, but
alternative commands exist to add preprocessor definitions
(:command:`target_compile_definitions` and :command:`add_definitions`) or
include directories (:command:`target_include_directories` and
:command:`include_directories`).  See documentation of the
:prop_tgt:`directory <COMPILE_OPTIONS>` and
:prop_tgt:` target <COMPILE_OPTIONS>` ``COMPILE_OPTIONS`` properties.

Arguments to ``add_compile_options`` may use "generator expressions" with
the syntax ``$<...>``.  See the :manual:`cmake-generator-expressions(7)`
manual for available expressions.  See the :manual:`cmake-buildsystem(7)`
manual for more on defining buildsystem properties.
