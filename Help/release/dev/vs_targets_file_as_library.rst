vs_targets_file_as_library
--------------------------

* :ref:`Visual Studio Generators` learned to treat files passed to
  :command:`target_link_libraries` whose names end in ``.targets``
  as MSBuild targets files to be imported into generated project files.
