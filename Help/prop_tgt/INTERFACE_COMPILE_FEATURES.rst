INTERFACE_COMPILE_FEATURES
--------------------------

List of public compile requirements for a library.

Targets may populate this property to publish the compiler features
required to compile against the headers for the target.  Consuming
targets can add entries to their own :prop_tgt:`COMPILE_FEATURES`
property such as ``$<TARGET_PROPERTY:foo,INTERFACE_COMPILE_FEATURES>``
to require the features specified in the interface of ``foo``.

Contents of ``INTERFACE_COMPILE_FEATURES`` may use "generator expressions"
with the syntax ``$<...>``.  See the :manual:`cmake-generator-expressions(7)`
manual for available expressions.
