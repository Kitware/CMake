main_dependency_diagnostic
--------------------------

* Listing the same input file as a MAIN_DEPENDENCY of a custom command
  can lead to broken build time behavior.  This is now diagnosed.
  See policy :policy:`CMP0057`.
