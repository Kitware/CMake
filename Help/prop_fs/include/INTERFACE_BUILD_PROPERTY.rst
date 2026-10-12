List of public |property_name| requirements for a file set.

File sets may populate this property to publish the |property_name|
required to compile the sources for the target.  The |command_name|
command populates this property.

When target dependencies are specified using :command:`target_link_libraries`,
CMake will read this property from file sets of all target dependencies to
determine the build properties of the consumer. These build properties are only
applied to the sources of the file sets. The other sources of the consumer are
unaffected.

.. |genex_subject| replace:: |PROPERTY_INTERFACE_NAME|
.. include:: /include/GENEX_NOTE.rst
