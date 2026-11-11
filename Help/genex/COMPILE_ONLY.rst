COMPILE_ONLY
------------

Syntax
^^^^^^

.. code-block:: cmake

  $<COMPILE_ONLY:...>

.. versionadded:: 3.27

Content of ``...``, when collecting :ref:`transitive compile properties
<Transitive Compile Properties>`, otherwise it is the empty string.  This is
intended for use in an :prop_tgt:`INTERFACE_LINK_LIBRARIES` and
:prop_tgt:`LINK_LIBRARIES` target properties, typically populated via the
:command:`target_link_libraries` command. Provides compilation usage
requirements without any linking requirements.

Use cases include header-only usage where all usages are known to not have
linking requirements (e.g., all-``inline`` or C++ template libraries).

Note that for proper evaluation of this expression requires policy
:policy:`CMP0099` to be set to ``NEW``.
