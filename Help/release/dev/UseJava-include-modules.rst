UseJava-include-modules
-----------------------

* The :module:`UseJava` module's :command:`add_jar` command now accepts a new
  option ``INCLUDE_MODULES`` that adds its arguments to the ``--module-path``
  argument to the Java compiler. This allows building JAR files that use JPMS
  modules in their build.
