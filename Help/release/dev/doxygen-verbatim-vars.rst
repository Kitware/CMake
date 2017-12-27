FindDoxygen
-----------

* The :command:`doxygen_add_docs` function of the :module:`FindDoxygen` module
  now supports a new ``DOXYGEN_VERBATIM_VARS`` list variable. Any
  ``DOXYGEN_...`` variable contained in that list will bypass the automatic
  quoting logic, leaving its contents untouched when transferring them to the
  output Doxyfile.
