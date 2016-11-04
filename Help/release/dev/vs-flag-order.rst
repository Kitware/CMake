vs-flag-order
-------------

* The :ref:`Visual Studio Generators` for VS 2010 and above now place
  per-source file flags after target-wide flags when they are classified
  as raw flags with no project file setting (``AdditionalOptions``).
  This behavior is more consistent with the ordering of flags produced
  by other generators, and allows flags on more-specific properties
  (per-source) to override those on more general ones (per-target).
