try_compile-project-platform-vars
---------------------------------

* The :command:`try_compile` command
  :ref:`whole-project <Try Compiling Whole Projects>` signature
  now propagates platform variables.  See policy :policy:`CMP0137`.

* The :variable:`CMAKE_TRY_COMPILE_NO_PLATFORM_VARIABLES` variable
  was added to tell the :command:`try_compile` command not to
  pass any platform variables to the test project.
