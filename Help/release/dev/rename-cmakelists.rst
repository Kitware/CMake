Option to specify alternate CMakeLists filename
-----------------------------------------------

* Adds :option:`cmake --project-file` option to specify an alternate filename
  for CMakeLists files.  This determines the top-level file processed when CMake
  is configured, and the file processed by :command:`add_subdirectory`. By
  default, this is ``CMakeLists.txt``. If set to anything else,
  ``CMakeLists.txt`` will be used as a fallback if the given file cannot be
  found within a project subdirectory. The use of alternate project file names
  is intended for temporary use by developers during an incremental transition
  and not for publication of a final product. CMake will always emit a warning
  when the project file is anything other than ``CMakeLists.txt``.
