cmake_path-IS_PREFIX-empty
--------------------------

* The :command:`cmake_path(IS_PREFIX)` command and the
  :genex:`$<PATH:IS_PREFIX>` generator expression no longer treat
  an empty path as a prefix of every path.  The :command:`if` command's
  ``PATH_IS_PREFIX`` operator follows the same rule.
  See policy :policy:`CMP0223`.
