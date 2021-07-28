CMAKE_<LANG>_LINKER_LAUNCHER
----------------------------

.. versionadded:: 3.21

Default value for :prop_tgt:`<LANG>_LINKER_LAUNCHER` target property. This
variable is used to initialize the property on each target as it is created.
This is done only when ``<LANG>`` is ``C``, ``CXX``, ``OBJC``, or ``OBJCXX``.

This variable is initialized to the :envvar:`CMAKE_<LANG>_LINKER_LAUNCHER`
environment variable if it is set.
