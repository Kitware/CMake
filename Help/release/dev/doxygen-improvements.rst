doxygen-improvements
--------------------

* The :module:`FindDoxygen` module learned to control Doxygen behavior using
  CMake variables and generate documentation via the newly added
  :command:`doxygen_add_docs` function. The Doxygen input file (``Doxyfile``)
  is automatically generated and doxygen is run as part of a custom target.
  A number of doxygen-related variables have been deprecated. Additional
  components can be specified to find optional tools: ``dot``, ``mscgen``
  and ``dia``.
