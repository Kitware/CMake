vs7-OutputDirectory
-------------------

* The :variable:`CMAKE_CFG_INTDIR` variable value for Visual Studio
  7, 8, and 9 is now ``$(ConfigurationName)`` instead of ``$(OutDir)``.
  This should have no effect on the intended use cases of the variable.

* With Visual Studio 7, 8, and 9 generators the value of the ``$(OutDir)``
  placeholder no longer evaluates to the configuration name.  Projects
  should use ``$(ConfigurationName)`` for that instead.
