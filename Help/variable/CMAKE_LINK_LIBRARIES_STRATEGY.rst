CMAKE_LINK_LIBRARIES_STRATEGY
-----------------------------

.. versionadded:: 3.31

Specify a strategy for ordering targets' direct link dependencies
on linker command lines.

The value of this variable initializes the :prop_tgt:`LINK_LIBRARIES_STRATEGY`
target property of targets as they are created.  Set that property directly
to specify a strategy for a single target.

CMake generates a target's link line using its :ref:`Target Link Properties`.
In particular, the :prop_tgt:`LINK_LIBRARIES` target property records the
target's direct link dependencies, typically populated by calls to
:command:`target_link_libraries`.  Indirect link dependencies are
propagated from those entries of :prop_tgt:`LINK_LIBRARIES` that name
library targets by following the transitive closure of their
:prop_tgt:`INTERFACE_LINK_LIBRARIES` properties.  CMake supports multiple
strategies for passing direct and indirect link dependencies to the linker.

Consider this example for the strategies below:

.. code-block:: cmake

  add_library(A STATIC ...)
  add_library(B STATIC ...)
  add_library(C STATIC ...)
  add_executable(main ...)
  target_link_libraries(B PRIVATE A)
  target_link_libraries(C PRIVATE A)
  target_link_libraries(main PRIVATE A B C)

The supported strategies are:

``PRESERVE_ORDER``
  Entries of :prop_tgt:`LINK_LIBRARIES` always appear first and in their
  original order.  Indirect link dependencies not satisfied by the
  original entries may be reordered and de-duplicated with respect to
  one another, but are always appended after the original entries.
  This may result in less efficient link lines, but gives projects
  control of ordering among independent entries.  Such control may be
  important when intermixing link flags with libraries, or when multiple
  libraries provide a given symbol.

  This is the default.

  In the above example, this strategy computes a link line for ``main``
  by starting with its original entries ``A B C``, and then appends
  another ``A`` to satisfy the dependencies of ``B`` and ``C`` on ``A``.
  The final order is ``A B C A``.

``REORDER``
  Entries of :prop_tgt:`LINK_LIBRARIES` may be reordered, de-duplicated,
  and intermixed with indirect link dependencies.  This may result in
  more efficient link lines, but does not give projects any control of
  ordering among independent entries.

  In the above example, this strategy computes a link line for ``main``
  by re-ordering its original entries ``A B C`` to satisfy the
  dependencies of ``B`` and ``C`` on ``A``.
  The final order is ``B C A``.

.. note::

  Regardless of the strategy used, the actual linker invocation for
  some platforms may de-duplicate entries based on linker capabilities.
  See policies :policy:`CMP0156` and :policy:`CMP0179`.
