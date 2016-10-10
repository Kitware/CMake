st2-exclude-patterns-variable
-----------------------------

* The :generator:`Sublime Text 2` extra generator no longer excludes the
  build tree from the ``.sublime-project`` when it is inside the source tree.
  The :variable:`CMAKE_SUBLIME_TEXT_2_EXCLUDE_BUILD_TREE` variable
  was added to control the behavior explicitly.
