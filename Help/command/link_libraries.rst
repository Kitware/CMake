link_libraries
--------------

Deprecated. Use the target_link_libraries() command instead.

Link libraries to all targets added later.

::

  link_libraries(library1 <debug | optimized> library2 ...)

Specify a list of libraries to be linked into any following targets
(typically added with the add_executable or add_library calls).  This
command is passed down to all subdirectories.  The debug and optimized
strings may be used to indicate that the next library listed is to be
used only for that specific type of build.
