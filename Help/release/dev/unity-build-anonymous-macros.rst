unity-build-anonymous-macros
----------------------------

* The :prop_tgt:`UNITY_BUILD_UNIQUE_ID` target property
  was added to support generation of an identifier that is
  unique per source file in unity builds.  It can help to
  resolve duplicate symbol problems with anonymous namespaces.
