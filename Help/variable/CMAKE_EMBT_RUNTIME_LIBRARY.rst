CMAKE_EMBT_RUNTIME_LIBRARY
--------------------------

.. versionadded:: 4.6

Select the runtime library used by the Embarcadero (``bcc64x``) toolchain.
This variable is used to initialize the :prop_tgt:`EMBT_RUNTIME_LIBRARY`
property on all targets as they are created.

The allowed values are:

.. include:: ../prop_tgt/include/EMBT_RUNTIME_LIBRARY-VALUES.rst

If this variable is not set then the :prop_tgt:`EMBT_RUNTIME_LIBRARY` target
property is not set automatically.
