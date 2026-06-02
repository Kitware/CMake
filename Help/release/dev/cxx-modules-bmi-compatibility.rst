cxx-modules-bmi-compatibility
-----------------------------

* Targets which provide ``PUBLIC`` or ``INTERFACE``-scoped ``CXX_MODULES``
  :ref:`file sets <File Sets>` will now generate
  :term:`synthetic targets <synthetic target>` when linked to by
  :term:`BMI`-incompatible consuming targets. These synthetic targets adopt
  the :prop_tgt:`COMPILE_FEATURES` and :prop_tgt:`COMPILE_OPTIONS`
  from the consumer, while all other relevant properties are inherited from the
  provider.
