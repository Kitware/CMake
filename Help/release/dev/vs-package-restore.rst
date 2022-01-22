vs-package-restore
------------------

* Targets with :prop_tgt:`VS_PACKAGE_REFERENCES` will now automatically attempt
  to restore the package references from NuGet. The cache variable
  :variable:`CMAKE_VS_NUGET_PACKAGE_RESTORE` was added to toggle automatic
  package restore off.

* :manual:`cmake(1)` gained the ``--resolve-package-references=<on|off|only>``
  command-line option to control automatic package restoration.
