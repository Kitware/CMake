INDEPENDENT_FILES
-----------------

.. versionadded:: 4.4

``INDEPENDENT_FILES`` is a boolean specifying that any :prop_sf:`GENERATED`
sources in the file set are not necessary for the compilation of other sources
in the same target. Stated another way, these files are "independent" and their
presence is never necessary for compilation of other sources (e.g., via
``#include``).

When this property is ``ON``, :ref:`Ninja Generators` will omit conservative
order-only dependencies that prevent a target's source files from compiling
before custom commands from the target's dependencies are finished, even if
those custom commands only produce sources independent from other sources in
the same target.
When this property is ``OFF``, :ref:`Ninja Generators` will apply conservative
order-only dependencies that prevent a target's source files from compiling
before custom commands from the target's dependencies are finished, even if
those custom commands only produce sources independent from other sources in
the same target.

If this property is not defined, the following default will be applied:
* ``ON`` for ``CXX_MODULES`` file set type.
* ``OFF`` for all other file set types.

In the case of the ``CXX_MODULES`` file set type, the only supported mode is
``ON``. So if the property is set to ``OFF``, it will be ignored.

In the case of the ``HEADERS`` file set type, if the property is ``ON``, an
author warning will be emit because this mode is not supported.

This property is effective only when the policy :policy:`CMP0154` is ``NEW``.
