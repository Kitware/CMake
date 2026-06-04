CMD_AUTHOR
----------

.. versionadded:: 4.4

.. diagnostic::
  :default: warn

  Warn about a build system's incorrect use of CMake, or of a CMake interface
  provided by a dependency.  This is the category triggered by
  :command:`message(AUTHOR_WARNING)`.  It is also the ancestor of many other
  diagnostic categories.

  The most important aspect of this category is that it represents issues with
  a project's build system which typically require alteration to the same.
  This is to say that users simply trying to build a project obtained elsewhere
  will typically not be interested in these warnings, except to perhaps report
  them to the project's developer(s).
