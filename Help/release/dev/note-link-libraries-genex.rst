note-link-libraries-genex
-------------------------

* In CMake 3.0 the :command:`target_link_libraries` command
  accidentally began allowing unquoted arguments to use
  :manual:`generator expressions <cmake-generator-expressions(7)>`
  containing a (``;`` separated) list within them.  For example::

    set(libs B C)
    target_link_libraries(A PUBLIC $<BUILD_INTERFACE:${libs}>)

  This is equivalent to writing::

    target_link_libraries(A PUBLIC $<BUILD_INTERFACE:B C>)

  and was never intended to work.  It did not work in CMake 2.8.12.
  Such generator expressions should be in quoted arguments::

    set(libs B C)
    target_link_libraries(A PUBLIC "$<BUILD_INTERFACE:${libs}>")

  CMake 3.1 again requires the quotes for this to work correctly.
