GenerateExportHeader-deprecated-msg
-----------------------------------

* The :module:`GenerateExportHeader` module's
  :command:`generate_export_header` command now defines
  ``<MACRO>_DEPRECATED_MSG(<message>)`` and the corresponding ``_EXPORT_MSG``
  and ``_NO_EXPORT_MSG`` macros, which add a message to the deprecation
  warning.  With the ``CXX_ATTRIBUTES`` option, deprecation uses the
  standard ``[[deprecated]]`` attribute as of C++14.
