CMAKE_LINK_LIBRARY_USING_<FEATURE>
----------------------------------

.. versionadded:: 3.24

This variable defines, for the specified ``FEATURE``, the expression expected
by the linker, regardless the linker language, when libraries are specified
using :genex:`LINK_LIBRARY` generator expression.

.. note::

  Feature names defined in all uppercase are reserved to CMake.

See also the associated variable
:variable:`CMAKE_LINK_LIBRARY_USING_<FEATURE>_SUPPORTED` and
:variable:`CMAKE_<LANG>_LINK_LIBRARY_USING_<FEATURE>` variable for the
definition of features dependent from the link language.

This variable will be used by :genex:`LINK_LIBRARY` generator expression if,
for the linker language, the variable
:variable:`CMAKE_<LANG>_LINK_LIBRARY_USING_<FEATURE>_SUPPORTED` is false or not
set.

.. include:: CMAKE_LINK_LIBRARY_USING_FEATURE.txt

Predefined Features
^^^^^^^^^^^^^^^^^^^

``CMake`` pre-defines some features of general interest:

.. include:: LINK_LIBRARY_PREDEFINED_FEATURES.txt
