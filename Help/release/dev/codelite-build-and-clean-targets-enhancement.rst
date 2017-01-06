codelite-build-and-clean-targets-enhancement
--------------------------------------------

* The :generator:`CodeLite` extra generator gained a new option
  set by the :variable:`CMAKE_CODELITE_USE_TARGETS` variable to
  change the generated project to have target-centric organization.
  The "build", "rebuild", and "clean" operations within CodeLite
  then work on a selected target rather than the whole workspace.
  (Note that the :generator:`Ninja` clean operation on a target
  includes its dependencies, though.)
