INTERFACE_INCLUDE_DIRECTORIES
-----------------------------

.. versionadded:: 4.4

.. |property_name| replace:: include directories
.. |command_name| replace:: :command:`set_property(FILE_SET)`
.. |PROPERTY_INTERFACE_NAME| replace:: ``INTERFACE_INCLUDE_DIRECTORIES``
.. include:: include/INTERFACE_BUILD_PROPERTY.rst

Include directories usage requirements commonly differ between the build-tree
and the install-tree.  The ``BUILD_INTERFACE`` and ``INSTALL_INTERFACE``
generator expressions can be used to describe separate usage requirements
based on the usage location.  Relative paths are allowed within the
``INSTALL_INTERFACE`` expression and are interpreted relative to the
installation prefix.  For example:

.. code-block:: cmake

  set_property(FILE_SET myfile_set TARGET mylib PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/mylib>
    $<INSTALL_INTERFACE:include/mylib>  # <prefix>/include/mylib
  )

Related properties:

* Use :prop_fs:`INTERFACE_COMPILE_DEFINITIONS` to pass additional preprocessor
  definitions.
* Use :prop_fs:`INTERFACE_COMPILE_OPTIONS` to pass additional compile flags.

Creating Relocatable Packages
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. |INTERFACE_PROPERTY_LINK| replace:: ``INTERFACE_INCLUDE_DIRECTORIES``
.. include:: /include/INTERFACE_INCLUDE_DIRECTORIES_WARNING.rst
