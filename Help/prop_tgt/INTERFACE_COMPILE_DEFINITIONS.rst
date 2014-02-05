INTERFACE_COMPILE_DEFINITIONS
-----------------------------

List of public compile definitions for a library.

Targets may populate this property to publish the compile definitions
required to compile against the headers for the target.  Consuming
targets can add entries to their own :prop_tgt:`COMPILE_DEFINITIONS`
property such as ``$<TARGET_PROPERTY:foo,INTERFACE_COMPILE_DEFINITIONS>``
to use the compile definitions specified in the interface of ``foo``.

Contents of ``INTERFACE_COMPILE_DEFINITIONS`` may use "generator expressions"
with the syntax ``$<...>``.  See the :manual:`cmake-generator-expressions(7)`
manual for available expressions.  See the :manual:`cmake-buildsystem(7)`
manual for more on defining buildsystem properties.
