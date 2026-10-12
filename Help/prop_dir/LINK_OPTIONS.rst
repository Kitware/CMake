LINK_OPTIONS
------------

.. versionadded:: 3.13

List of options to use for the link step of shared library, module
and executable targets as well as the device link step.

This property holds a :ref:`semicolon-separated list <CMake Language Lists>` of options
given so far to the :command:`add_link_options` command.

This property is used to initialize the :prop_tgt:`LINK_OPTIONS` target
property when a target is created, which is used by the generators to set
the options for the compiler.

.. |genex_subject| replace:: ``LINK_OPTIONS``
.. include:: ../include/GENEX_NOTE.rst
