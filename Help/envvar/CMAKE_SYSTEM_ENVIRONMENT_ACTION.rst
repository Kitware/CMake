CMAKE_SYSTEM_ENVIRONMENT_ACTION
-------------------------------

.. versionadded:: 4.5

.. include:: include/ENV_VAR.rst

Specify the action taken by :manual:`cmake(1)` when the value of
:envvar:`CMAKE_SYSTEM_ENVIRONMENT_ID` in the environment does not match the
cached value. If not set, the default is ``WARN``.

The value may be one of:

``IGNORE``
  No action is taken.

``WARN``
  A warning is emitted to the user.

``REFRESH``
  The cache is automatically refreshed, as if by :cmake-option:`--fresh`.
  Take care when setting this option, as all cache variables not specified in
  a configure preset or the current invocation of :manual:`cmake(1)` will be
  lost.
