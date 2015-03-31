UseSWIG-no-MAIN_DEPENDENCY
--------------------------

* The :module:`UseSWIG` module ``SWIG_ADD_MODULE`` macro no
  longer attaches the swig invocation custom command to the
  ``.i`` source file in IDE projects.  This is because only
  one custom command can be safely attached to a given source
  file, and adding multiple modules with the same ``.i`` file
  for different languages requires more than one such command.
