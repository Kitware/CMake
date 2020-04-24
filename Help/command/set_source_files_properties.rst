set_source_files_properties
---------------------------

Source files can have properties that affect how they are built.

.. code-block:: cmake

  set_source_files_properties([file1 [file2 [...]]]
                              [<TARGET_DIRECTORY ... | DIRECTORY ...>]
                              PROPERTIES prop1 value1
                              [prop2 value2 [...]])

Sets properties associated with source files using a key/value paired
list.

Note that source file properties are by default visible only to
targets added in the same directory (``CMakeLists.txt``).

The file properties can be made visible in a different directory by specifying
one of the additional options: ``TARGET_DIRECTORY`` or ``DIRECTORY``.

``DIRECTORY`` takes a list of processed directories paths, and sets the file
properties in those directory scopes.

``TARGET_DIRECTORY`` takes a list of existing targets. The file
properties will be set in these targets' directory scopes.

See also the :command:`set_property(SOURCE)` command.

See :ref:`Source File Properties` for the list of properties known
to CMake.  Source file properties are visible only to targets added
in the same directory (``CMakeLists.txt``).
