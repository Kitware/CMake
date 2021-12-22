vs-csharp-dotnet-sdk
--------------------

* The :ref:`Visual Studio Generators` for VS 2019 and above learned to
  support .NET SDK-style project files (``.csproj``) for C# projects.
  See the :prop_tgt:`DOTNET_SDK` target property and corresponding
  :variable:`CMAKE_DOTNET_SDK` variable.
  However, this version of CMake does not yet support using
  :command:`add_custom_command` in .NET SDK-style projects.
