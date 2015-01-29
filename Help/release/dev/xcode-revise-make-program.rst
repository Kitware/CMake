xcode-revise-make-program
-------------------------

* The :generator:`Xcode` generator no longer requires a value for
  the :variable:`CMAKE_MAKE_PROGRAM` variable to be located up front.
  It now locates ``xcodebuild`` when needed at build time.
