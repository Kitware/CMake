vs-native-csharp-support
------------------------

* The :ref:`Visual Studio Generators` for VS 2010 and above
  learned to support the C# language. C# assemblies and
  programs can be added just like common C++ targets using
  the :command:`add_library` and :command:`add_executable`
  commands. Referencing between several C# targets in the same
  source tree is done by :command:`target_link_libraries` like
  for C++. Referencing to system or 3rd party assemblies is
  done by the target properties :prop_tgt:`VS_DOTNET_REFERENCE_<refname>`
  and :prop_tgt:`VS_DOTNET_REFERENCES`.

* C# as a language can be enabled using :command:`enable_language`
  or :command:`project` with ``CSharp``. It is not enabled by
  default.

* Flag variables, target properties and other configuration
  that specifically targets C# contains ``CSharp`` as a part of
  their names.

* More finetuning of C# targets can be done using target
  properties. Specifically the Visual Studio related target
  properties (``VS_*``) are worth a look (for setting toolset
  versions, root namespaces, assembly icons, ...).
