TARGET_PROPERTY
---------------

.. genex:: $<TARGET_PROPERTY:prop>
   :target: TARGET_PROPERTY:prop

.. |genex-family| replace:: :ref:`genex-family-target-dependent-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_PROPERTY:tgt,prop>

Value of the property ``prop`` on the target ``tgt``, or empty if the property
is not set.

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on.

.. versionchanged:: 3.26
  When encountered during evaluation of :ref:`Target Usage Requirements`,
  typically in an ``INTERFACE_*`` target property, lookup of the ``tgt``
  name occurs in the directory of the target specifying the requirement,
  rather than the directory of the consuming target for which the
  expression is being evaluated.

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_PROPERTY:prop>

Value of the property ``prop`` on the target for which the expression is being
evaluated, or empty if the property is not set. Note that for generator
expressions in :ref:`Target Usage Requirements` this is the consuming target
rather than the target specifying the requirement. For generator expressions in
:ref:`source file properties <Source File Properties>`, this is the target
compiling the source file, so a source file shared by several targets can be
compiled differently by each.
