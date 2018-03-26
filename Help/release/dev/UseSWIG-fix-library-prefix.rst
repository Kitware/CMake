UseSWIG-fix-library-prefix
--------------------------

* The :module:`UseSWIG` module :command:`swig_add_library` command
  (and legacy ``swig_add_module`` command) now set the prefix of
  Java modules to ``""`` for MINGW, MSYS, and CYGWIN environments.
