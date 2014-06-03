INTERFACE_INCLUDE_DIRECTORIES
-----------------------------

List of public include directories for a library.

The :command:`target_include_directories` command populates this property
with values given to the ``PUBLIC`` and ``INTERFACE`` keywords.  Projects
may also get and set the property directly.

Targets may populate this property to publish the include directories
required to compile against the headers for the target.  Consuming
targets can add entries to their own :prop_tgt:`INCLUDE_DIRECTORIES`
property such as ``$<TARGET_PROPERTY:foo,INTERFACE_INCLUDE_DIRECTORIES>``
to use the include directories specified in the interface of ``foo``.

Contents of ``INTERFACE_INCLUDE_DIRECTORIES`` may use "generator expressions"
with the syntax ``$<...>``.  See the :manual:`cmake-generator-expressions(7)`
manual for available expressions.  See the :manual:`cmake-buildsystem(7)`
manual for more on defining buildsystem properties.

Include directories usage requirements commonly differ between the build-tree
and the install-tree.  The ``BUILD_INTERFACE`` and ``INSTALL_INTERFACE``
generator expressions can be used to describe separate usage requirements
based on the usage location.  Relative paths are allowed within the
``INSTALL_INTERFACE`` expression and are interpreted relative to the
installation prefix.  For example:

.. code-block:: cmake

  set_property(TARGET mylib APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/mylib>
    $<INSTALL_INTERFACE:include/mylib>  # <prefix>/include/mylib
    )
