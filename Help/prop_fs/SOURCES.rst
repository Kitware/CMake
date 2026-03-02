SOURCES
-------

This specifies the list of paths to sources for the file set.
The :command:`target_sources` command sets or adds to the ``SOURCES`` file set
property for the file sets defined with the ``PRIVATE`` or ``PUBLIC`` keyword
and is the usual way to manipulate it.

Contents of ``SOURCES`` may use
:manual:`generator expressions <cmake-generator-expressions(7)>`.

Each file must be in one of the base directories, or a subdirectory of one of
the base directories. If relative paths are specified, they are considered
relative to the target's source directory.

The following behavior applies for the ``SOURCES`` and
:prop_fs:`INTERFACE_SOURCES` file set properties, dependent on the value of the
:prop_fs:`SCOPE` file set property:

``PRIVATE``
  Only the ``SOURCES`` property can be set. Any change to
  the :prop_fs:`INTERFACE_SOURCES` property will be ignored.

``PUBLIC``
  ``SOURCES`` and :prop_fs:`INTERFACE_SOURCES` properties will always have the
  same content.

``INTERFACE``
  Only the :prop_fs:`INTERFACE_SOURCES` property can be set. Any change to
  ``SOURCES`` property will be ignored.
