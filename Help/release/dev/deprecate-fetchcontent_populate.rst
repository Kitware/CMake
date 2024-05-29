deprecate-fetchcontent_populate
-------------------------------

* Calling :command:`FetchContent_Populate` with just the name of a
  dependency is now deprecated. Projects should call
  :command:`FetchContent_MakeAvailable` instead. See policy :policy:`CMP0169`.
  Calling :command:`FetchContent_Populate` with full population details
  rather than just a dependency name remains fully supported.
