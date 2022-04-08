vs_dotnet_startup_object_support
--------------------------------

* The :prop_tgt:`VS_DOTNET_STARTUP_OBJECT` target property was added to
  tell :ref:`Visual Studio Generators` which startup class shall be used
  when the program or project is executed. This is necessary when more
  than one ``static void Main(string[])`` function signature is available
  in a managed .NET project.
