cmake-e-tar-error-handling
--------------------------

* The :manual:`cmake(1)` ``-E tar`` tool now parses all flags, and if an
  invalid flag was provided, a warning is issued.
* The :manual:`cmake(1)` ``-E tar`` tool now displays an error if no action
  flag was specified, along with a list of possible actions: ``t`` (list),
  ``c`` (create) or ``x`` (extract).
