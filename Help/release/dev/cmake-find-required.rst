cmake-find-required
-------------------

* The :variable:`CMAKE_FIND_REQUIRED` variable was added to tell
  :command:`find_package`, :command:`find_path`, :command:`find_file`,
  :command:`find_library`, and :command:`find_program` to be ``REQUIRED``
  by default.  The commands also gained an ``OPTIONAL`` keyword to ignore
  the variable for a specific call.
