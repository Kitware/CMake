DEPENDS_EXPLICIT_ONLY
---------------------

.. versionadded:: 4.5

``DEPENDS_EXPLICIT_ONLY`` is a boolean indicating that the rule's ``DEPENDS``
argument represents all files required by the command and implicit dependencies
are not required.

If not defined, the :variable:`CMAKE_ADD_CUSTOM_COMMAND_DEPENDS_EXPLICIT_ONLY`
will be used during the instantiations of the rule.
