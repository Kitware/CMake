COMPILE_OPTIONS
---------------

List of options to pass to the compiler.

This property holds a :ref:`semicolon-separated list <CMake Language Lists>` of options
given so far to the :command:`add_compile_options` command.

This property is used to initialize the :prop_tgt:`COMPILE_OPTIONS` target
property when a target is created, which is used by the generators to set
the options for the compiler.

.. |genex_subject| replace:: ``COMPILE_OPTIONS``
.. include:: ../include/GENEX_NOTE.rst
