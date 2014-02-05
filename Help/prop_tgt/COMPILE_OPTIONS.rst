COMPILE_OPTIONS
---------------

List of options to pass to the compiler.

This property specifies the list of options specified so far for this
property.

This property is intialized by the :prop_dir:`COMPILE_OPTIONS` directory
property, which is used by the generators to set the options for the
compiler.

Contents of ``COMPILE_OPTIONS`` may use "generator expressions" with the
syntax ``$<...>``.  See the :manual:`cmake-generator-expressions(7)` manual
for available expressions.  See the :manual:`cmake-buildsystem(7)` manual
for more on defining buildsystem properties.
