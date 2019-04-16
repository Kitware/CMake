doxygen-add-docs-USE_STAMP_FILE
-------------------------------

* The :command:`doxygen_add_docs` command from the :module:`FindDoxygen`
  module gained a new ``USE_STAMP_FILE`` option.  When this option present,
  the custom target created by the command will only re-run Doxygen if any
  of the source files have changed since the last successful run.
