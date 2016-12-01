try_compile-lang-std
--------------------

* The :command:`try_compile` command source file signature gained new options
  to specify the language standard to use in the generated test project.

* The :command:`try_compile` command source file signature now honors
  language standard variables like :variable:`CMAKE_CXX_STANDARD`.
  See policy :policy:`CMP0067`.
