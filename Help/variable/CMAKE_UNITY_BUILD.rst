CMAKE_UNITY_BUILD
-----------------

Initializes the :prop_tgt:`UNITY_BUILD` target property on targets
as they are created.  Set to ``ON`` to batch compilation of multiple
sources within each target.  This feature is known as "Unity build",
or "Jumbo build".  By default this variable is not set and so does
not enable unity builds on targets.

.. note::
  This option currently does not work well in combination with
  the :variable:`CMAKE_EXPORT_COMPILE_COMMANDS` variable.
