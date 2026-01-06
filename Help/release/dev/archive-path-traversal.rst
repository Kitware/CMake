archive-path-traversal
----------------------

* The :manual:`cmake(1)` :option:`-E tar <cmake-E tar>` command-line tool,
  and the :command:`file(ARCHIVE_EXTRACT)` command, now reject archive
  entries whose paths are absolute or contain ``..`` path traversal
  components.
