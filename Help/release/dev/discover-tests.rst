discover-tests
--------------

* The :command:`discover_tests` command was added to support registering tests
  discovered at test time by :manual:`ctest(1)`. It runs a user-provided
  discovery command, parses its output, and generates test names, arguments,
  and properties from regular-expression captures.
