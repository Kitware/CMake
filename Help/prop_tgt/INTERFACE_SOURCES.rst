INTERFACE_SOURCES
-----------------

List of interface sources to pass to the compiler.

Targets may populate this property to publish the sources
for consuming targets to compile.  Consuming
targets can add entries to their own :prop_tgt:`SOURCES` property
such as ``$<TARGET_PROPERTY:foo,INTERFACE_SOURCES>`` to use the
sources specified in the interface of ``foo``.

Contents of ``INTERFACE_SOURCES`` may use "generator expressions"
with the syntax ``$<...>``.  See the :manual:`cmake-generator-expressions(7)`
manual for available expressions.  See the :manual:`cmake-buildsystem(7)`
manual for more on defining buildsystem properties.
