add_compile_options
-------------------

Add options to the compilation of source files.

.. code-block:: cmake

  add_compile_options(<option> ...)

Adds options to the :prop_dir:`COMPILE_OPTIONS` directory property.
These options are used when compiling targets from the current
directory and below.

Arguments
^^^^^^^^^

.. |command_name| replace:: ``add_compile_options``
.. include:: GENEX_NOTE.txt

.. include:: OPTIONS_SHELL.txt

Example
^^^^^^^

Since different compilers support different options, a typical use of
this command is in a compiler-specific conditional clause:

.. code-block:: cmake

  if (MSVC)
      # warning level 4
      add_compile_options(/W4)
  else()
      # additional warnings
      add_compile_options(-Wall -Wextra -Wpedantic)
  endif()

To set per-language options, use the :genex:`$<COMPILE_LANGUAGE>`
or :genex:`$<COMPILE_LANGUAGE:languages>` generator expressions.

See Also
^^^^^^^^

* This command can be used to add any options. However, for
  adding preprocessor definitions and include directories it is recommended
  to use the more specific commands :command:`add_compile_definitions`
  and :command:`include_directories`.

* The command :command:`target_compile_options` adds target-specific options.

* The source file property :prop_sf:`COMPILE_OPTIONS` adds options to one
  source file.
