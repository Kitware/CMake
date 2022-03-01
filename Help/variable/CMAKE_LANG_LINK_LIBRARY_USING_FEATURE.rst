CMAKE_<LANG>_LINK_LIBRARY_USING_<FEATURE>
-----------------------------------------

.. versionadded:: 3.24

This variable defines, for the specified ``<FEATURE>`` and the linker language
``<LANG>``, the expression expected by the linker when libraries are specified
using :genex:`LINK_LIBRARY` generator expression.

.. note::

  * Feature names can contain Latin letters, digits and undercores.
  * Feature names defined in all uppercase are reserved to CMake.

See also the associated variable
:variable:`CMAKE_<LANG>_LINK_LIBRARY_USING_<FEATURE>_SUPPORTED` and
:variable:`CMAKE_LINK_LIBRARY_USING_<FEATURE>` variable for the definition of
features independent from the link language.

.. include:: CMAKE_LINK_LIBRARY_USING_FEATURE.txt

Predefined Features
^^^^^^^^^^^^^^^^^^^

``CMake`` pre-defines some features of general interest:

.. include:: LINK_LIBRARY_PREDEFINED_FEATURES.txt
