file-remove-no-empty
--------------------

* The :command:`file(REMOVE)` and :command:`file(REMOVE_RECURSE)` commands
  were changed to ignore empty arguments with a warning instead of treating
  them as a relative path and removing the contents of the current directory.
