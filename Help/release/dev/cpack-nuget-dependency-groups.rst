cpack-nuget-dependency-groups
-----------------------------

* The :cpack_gen:`CPack NuGet Generator` can now generate dependency groups
  for framework-specific dependencies. The :variable:`CPACK_NUGET_PACKAGE_TFMS`
  was added to specify a list of framework TFMs for which groups should be
  generated.
