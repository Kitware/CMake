ENABLE_EXPORTS
--------------

Specify whether an executable or a shared library exports symbols.

Normally an executable does not export any symbols because it is the
final program.  It is possible for an executable to export symbols to
be used by loadable modules.  When this property is set to true CMake
will allow other targets to "link" to the executable with the
:command:`target_link_libraries` command.  On all platforms a target-level
dependency on the executable is created for targets that link to it.
Handling of the executable on the link lines of the loadable modules
varies by platform:

* On Windows-based systems (including Cygwin) an "import library" is
  created along with the executable to list the exported symbols.
  Loadable modules link to the import library to get the symbols.

* On macOS, loadable modules link to the executable itself using the
  ``-bundle_loader`` flag.

* On AIX, a linker "import file" is created along with the executable
  to list the exported symbols for import when linking other targets.
  Loadable modules link to the import file to get the symbols.

* On other platforms, loadable modules are simply linked without
  referencing the executable since the dynamic loader will
  automatically bind symbols when the module is loaded.

This property is initialized by the value of the variable
:variable:`CMAKE_EXECUTABLE_ENABLE_EXPORTS` if it is set when an executable
target is created.

.. versionadded:: 3.27
  On macOS, to link with a shared library (standard one as well as framework),
  a linker import file (e.g. a text-based stubs file, with ``.tbd`` extension)
  can be used instead of the shared library itself.

The generation of these linker import files, as well as the consumption, is
controlled by this property. When this property is set to true, CMake will
generate a ``.tbd`` file for each shared library created by
:command:`add_library` command. This allow other targets to use this ``.tbd``
file to link to the library with the :command:`target_link_libraries`
command.

.. note::

  For compatibility purpose, this property will be ignored if
  :prop_tgt:`XCODE_ATTRIBUTE_GENERATE_TEXT_BASED_STUBS <XCODE_ATTRIBUTE_<an-attribute>>`
  target property or the
  :variable:`CMAKE_XCODE_ATTRIBUTE_GENERATE_TEXT_BASED_STUBS <CMAKE_XCODE_ATTRIBUTE_<an-attribute>>`
  variable is set to ``NO``.

This property is initialized by the value of the variable
:variable:`CMAKE_SHARED_LIBRARY_ENABLE_EXPORTS` if it is set when a shared
library target is created.
