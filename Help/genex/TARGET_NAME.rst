TARGET_NAME
-----------

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_NAME:tgt>

The target name ``tgt`` as written.  This marks ``tgt`` as being the name of a
target wherever it appears inside a larger expression.  When the property
holding that expression is exported with a namespace by
:command:`install(EXPORT)` or :command:`export`, a marked target name is
rewritten to its namespaced imported target name (e.g. ``mylib::mylib``), the
same way a bare target name in a link-library list would be.  Marking is needed
when a target name is nested inside another generator expression that the export
machinery does not otherwise descend into, such as :genex:`LINK_LIBRARY` or
:genex:`LINK_GROUP`, and also when exporting targets to multiple dependent
export sets.

For example, a target name used inside :genex:`LINK_LIBRARY` is exported
verbatim unless it is marked:

.. code-block:: cmake

  # Exported as bare "mylib"; a consumer would see an unresolved -lmylib.
  target_link_libraries(myotherlib PUBLIC
    $<LINK_LIBRARY:WHOLE_ARCHIVE,mylib>)

  # Marked, so export rewrites it to the namespaced "mylib::mylib".
  target_link_libraries(myotherlib PUBLIC
    $<LINK_LIBRARY:WHOLE_ARCHIVE,$<TARGET_NAME:mylib>>)

The ``tgt`` text must be a literal name of a target; it may not contain
generator expressions.  During ordinary evaluation the target does not have to
exist, but when the expression is exported the marked name must resolve to a
target reachable from the export.
