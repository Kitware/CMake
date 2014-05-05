variable_watch-no-allowed-access
--------------------------------

* Callbacks established by the :command:`variable_watch` command will no
  longer receive the ``ALLOWED_UNKNOWN_READ_ACCESS`` access type when
  the undocumented ``CMAKE_ALLOW_UNKNOWN_VARIABLE_READ_ACCESS`` variable is
  set.  Uninitialized variable accesses will always be reported as
  ``UNKNOWN_READ_ACCESS``.
