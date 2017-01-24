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

* More finetuning of C# targets can be done using target and source
  file properties. Specifically the Visual Studio related target
  properties (``VS_*``) are worth a look (for setting toolset
  versions, root namespaces, assembly icons, ...).

* **Auto-"linking"** in .csproj files: In C#/.NET development with
  Visual Studio there is a number of visual editors used which
  generate code. Both the generated files and the ones edited
  with the UI are connected in the ``.csproj`` file using
  ``<DependentUpon>`` tags. If CMake finds within a C# project
  any source file with extension ``.Designer.cs`` or ``.xaml.cs``,
  it checks sibling files with extension ``.xaml``, ``.settings``,
  ``.resx`` or ``.cs`` and establishes the dependency connection.
