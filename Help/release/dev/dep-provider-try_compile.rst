dep-provider-try_compile
------------------------

* The :prop_gbl:`PROPAGATE_TOP_LEVEL_INCLUDES_TO_TRY_COMPILE` global property
  can be used to propagate :variable:`CMAKE_PROJECT_TOP_LEVEL_INCLUDES` into
  :command:`try_compile` calls that use the
  :ref:`whole-project signature <Try Compiling Whole Projects>`.
  This is primarily intended as a way for dependency providers to be enabled
  in such :command:`try_compile` calls.
