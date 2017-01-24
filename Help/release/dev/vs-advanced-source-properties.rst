vs-advanced-source-properties
-----------------------------

* The :ref:`Visual Studio Generators` for VS 2010 and above
  learned some more source file properties:

  - :prop_sf:`VS_RESOURCE_GENERATOR` (C# only): allows setting the resource
    generator
  - :prop_sf:`VS_COPY_TO_OUT_DIR`: parameter to set if file should be copied
    to output directory (values: ``Never``, ``Always``, ``PreserveNewest``)
  - :prop_sf:`VS_INCLUDE_IN_VSIX`: boolean property to include file include
    Visual Studio extension package
