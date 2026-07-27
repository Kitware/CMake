test-include-files-genex
------------------------

* The :prop_dir:`TEST_INCLUDE_FILES` and :prop_dir:`TEST_INCLUDE_FILE`
  directory properties now support :manual:`generator expressions
  <cmake-generator-expressions(7)>` in the include path.  On multi-config
  generators a ``$<CONFIG>``-parameterized path can select a different script
  per configuration.
