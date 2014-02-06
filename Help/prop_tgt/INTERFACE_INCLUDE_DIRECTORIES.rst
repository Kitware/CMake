INTERFACE_INCLUDE_DIRECTORIES
-----------------------------

List of public include directories for a library.

Targets may populate this property to publish the include directories
required to compile against the headers for the target.  Consuming
targets can add entries to their own :prop_tgt:`INCLUDE_DIRECTORIES`
property such as ``$<TARGET_PROPERTY:foo,INTERFACE_INCLUDE_DIRECTORIES>``
to use the include directories specified in the interface of ``foo``.

Contents of ``INTERFACE_INCLUDE_DIRECTORIES`` may use "generator expressions"
with the syntax ``$<...>``.  See the :manual:`cmake-generator-expressions(7)`
manual for available expressions.  See the :manual:`cmake-buildsystem(7)`
manual for more on defining buildsystem properties.
