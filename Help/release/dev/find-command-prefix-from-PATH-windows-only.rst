find-command-prefix-from-PATH-windows-only
------------------------------------------

* The :command:`find_library`, :command:`find_path`, and :command:`find_file`
  commands no longer search in installation prefixes derived from the ``PATH``
  environment variable on non-Windows platforms.  This behavior was added in
  CMake 3.3 to support Windows hosts but has proven problematic on UNIX hosts.
  Users that keep some ``<prefix>/bin`` directories in the ``PATH`` just for
  their tools do not necessarily want any supporting ``<prefix>/lib``
  directories searched.  One may set the ``CMAKE_PREFIX_PATH`` environment
  variable with a :ref:`;-list <CMake Language Lists>` of prefixes that are
  to be searched.
