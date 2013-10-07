LINK_LIBRARIES
--------------

List of direct link dependencies.

This property specifies the list of libraries or targets which will be
used for linking.  In addition to accepting values from the
target_link_libraries command, values may be set directly on any
target using the set_property command.

The target property values are used by the generators to set the link
libraries for the compiler.  See also the target_link_libraries
command.

Contents of LINK_LIBRARIES may use "generator expressions" with the syntax
"$<...>".  See the :manual:`cmake-generator-expressions(7)` manual for
available expressions.
