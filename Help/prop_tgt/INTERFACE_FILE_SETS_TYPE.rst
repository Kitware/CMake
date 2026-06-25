INTERFACE_FILE_SETS_<TYPE>
--------------------------

.. versionadded:: 4.5

Read-only list of the target's ``INTERFACE`` and ``PUBLIC`` file sets of type
``<TYPE>``.

See Also
^^^^^^^^

Related properties:

* :prop_tgt:`FILE_SETS_<TYPE>` to list the target's ``PRIVATE`` file sets
* :prop_tgt:`FILE_SET_TYPES` to list the target's file set types
* :prop_tgt:`INTERFACE_HEADER_SETS` to list the target's ``INTERFACE`` file
  sets of type ``HEADER``
* :prop_tgt:`INTERFACE_SOURCE_SETS` to list the target's ``INTERFACE`` file
  sets of type ``SOURCE``
* :prop_tgt:`INTERFACE_CXX_MODULE_SETS` to list the target's ``INTERFACE`` file
  sets of type ``CXX_MODULES``

Related commands:

* :command:`target_sources` to define file sets
