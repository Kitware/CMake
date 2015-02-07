makefile-progress-improvements
------------------------------

* With Makefile generators, the build-time progress output has been improved.
  It no longer mixes progress and build rule messages during parallel builds.
  The link rule messages now have progress and are displayed as bold green
  instead of bold red (since red is often associated with an error message).
