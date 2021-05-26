link-objects-first
------------------

* :command:`target_link_libraries` calls referencing object libraries
  via the :genex:`TARGET_OBJECTS` generator expression now place the
  object files before all libraries on the link line, regardless of
  their specified order.  See documentation on
  :ref:`Linking Object Libraries via \$\<TARGET_OBJECTS\>` for details.
