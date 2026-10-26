GenerateExportHeader-cxx-attributes
-----------------------------------

* The :module:`GenerateExportHeader` module's
  :command:`generate_export_header` command gained a ``CXX_ATTRIBUTES``
  option to define the export, no-export and deprecation macros as C++11
  attributes, which, unlike the compiler-specific spellings, can be combined
  with other C++11 attributes.
