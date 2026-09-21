COMPILE_OPTIONS
---------------

List of options to pass to the compiler.

This property holds a :ref:`semicolon-separated list <CMake Language Lists>`
of options specified so far for its rule.  Use the
:command:`set_property(RULE)` command to append more options.

Contents of ``COMPILE_OPTIONS`` may use :manual:`generator expressions
<cmake-generator-expressions(7)>` with the syntax ``$<...>``. See the
:manual:`cmake-buildsystem(7)` manual for more on defining buildsystem
properties.

.. include:: ../command/include/OPTIONS_SHELL.rst
