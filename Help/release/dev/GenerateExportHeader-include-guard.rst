GenerateExportHeader-include-guard
----------------------------------

* The :module:`GenerateExportHeader` module learned an optional
  ``INCLUDE_GUARD_NAME`` parameter to change the name of the include guard
  symbol written to the generated export header.
  Additionally, it now adds a comment after the closing ``#endif`` on the
  generated export header's include guard.
