debugger-working-directory
--------------------------

* The :variable:`CMAKE_DEBUGGER_WORKING_DIRECTORY` was added to
  initialize the corresponding target property.

* The :prop_tgt:`DEBUGGER_WORKING_DIRECTORY` target property was added
  to tell generators what debugger working directory should be set for
  the target.

* The :manual:`cmake-file-api(7)` "codemodel" version 2 ``version`` field has
  been updated to 2.8.

* The :manual:`cmake-file-api(7)` "codemodel" version 2 "target" object gained
  a new "debugger" field.
