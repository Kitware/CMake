get_source_file_property
------------------------

Get a property for a source file.

.. code-block:: cmake

  get_source_file_property(<variable> <file>
                           [DIRECTORY <dir> | TARGET_DIRECTORY <target>]
                           <property>)

Gets a property from a source file.  The value of the property is
stored in the specified ``<variable>``.  If the source property is not found,
the behavior depends on whether it has been defined to be an ``INHERITED``
property or not (see :command:`define_property`).  Non-inherited properties
will set ``variable`` to ``NOTFOUND``, whereas inherited properties will search
the relevant parent scope as described for the :command:`define_property`
command and if still unable to find the property, ``variable`` will be set to
an empty string.

By default, the source file's property will be read from the current source
directory's scope.

.. versionadded:: 3.18
  Directory scope can be overridden with one of the following sub-options:

  ``DIRECTORY <dir>``
    The source file property will be read from the ``<dir>`` directory's
    scope.  CMake must already know about that source directory, either by
    having added it through a call to :command:`add_subdirectory` or ``<dir>``
    being the top level source directory.  Relative paths are treated as
    relative to the current source directory.

  ``TARGET_DIRECTORY <target>``
    The source file property will be read from the directory scope in which
    ``<target>`` was created (``<target>`` must therefore already exist).

Use :command:`set_source_files_properties` to set property values.  Source
file properties usually control how the file is built. One property that is
always there is :prop_sf:`LOCATION`.

.. note::

  The :prop_sf:`GENERATED` source file property may be globally visible.
  See its documentation for details.

See Also
^^^^^^^^

* :command:`define_property`
* the more general :command:`get_property` command
* :command:`set_source_files_properties`
