CMAKE_SYSTEM_ENVIRONMENT_ID
---------------------------

.. versionadded:: 4.5

.. include:: include/ENV_VAR.rst

Externally-defined environment identifier that is cached. The value is not
interpreted by :manual:`cmake(1)` and is a hint about the state of the
environment during the first configure. When the value changes,
:manual:`cmake(1)` by default emits a warning to the user indicating that the
environment has changed and introspection results may be out of date.

The default warning behavior can be modified by setting
:envvar:`CMAKE_SYSTEM_ENVIRONMENT_ACTION`.
