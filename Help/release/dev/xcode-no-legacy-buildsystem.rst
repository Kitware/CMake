xcode-no-legacy-buildsystem
---------------------------

* The :generator:`Xcode` generator will now issue a fatal error if
  the Legacy Build System has been selected for Xcode 14 and
  newer. Those Xcode versions dropped support for the Legacy Build
  System and expect the project being set-up for their current
  Build System.
