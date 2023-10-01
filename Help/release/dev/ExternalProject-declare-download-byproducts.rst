ExternalProject-declare-download-byproducts
-------------------------------------------

* The :module:`ExternalProject` module now declares ``BYPRODUCTS`` for the
  downloaded file for generated ``download`` steps. Previously, if multiple
  external projects downloaded to the same file, hash verification could fail.
  Now, when using the :ref:`Ninja Generators`, this scenario is detected and
  Ninja will raise an error stating that multiple rules generate the same file.
