CMAKE_AUTOMOC_MACRO_NAMES
----------------------------

Additional macro names used by :variable:`CMAKE_AUTOMOC`
to determine if a C++ file needs to be processed by ``moc``.

This variable is used to initialize the :prop_tgt:`AUTOMOC_MACRO_NAMES`
property on all the targets. See that target property for additional
information.

By default it is empty.

Example
-------
Let CMake know that source files that contain ``CUSTOM_MACRO`` must be ``moc``
processed as well::

  set(CMAKE_AUTOMOC ON)
  set(CMAKE_AUTOMOC_MACRO_NAMES "CUSTOM_MACRO")
