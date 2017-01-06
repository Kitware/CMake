vs-dotnet-references
--------------------

* The :ref:`Visual Studio Generators` for VS 2010 and above can
  now handle .NET references with hintpaths. For this the new
  target property group :prop_tgt:`VS_DOTNET_REFERENCE_<refname>`
  was introduced. The ``<refname>`` part of the property name will
  be the name of the reference, the value will be the actual
  path to the assembly.

* Copying of referenced assemblies to the output directory can
  now be disabled using the target property
  :prop_tgt:`VS_DOTNET_REFERENCES_COPY_LOCAL`.
