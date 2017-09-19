get_filename_component-fix-program-split
----------------------------------------

* The :command:`get_filename_component` ``PROGRAM`` mode semantics
  have been revised to not tolerate unquoted spaces in the path
  to the program while also accepting arguments.  While technically
  incompatible with the old behavior, it is expected that behavior
  under typical use cases with properly-quoted command-lines has
  not changed.
