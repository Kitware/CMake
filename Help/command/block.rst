block
-----

.. versionadded:: 3.25

Evaluate a group of commands with a dedicated variable and/or policy scope.

.. code-block:: cmake

  block([SCOPE_FOR (POLICIES|VARIABLES)] [PROPAGATE <var-name>...])
    <commands>
  endblock()

All commands between ``block()`` and the matching :command:`endblock` are
recorded without being invoked.  Once the :command:`endblock` is evaluated, the
recorded list of commands is invoked inside the requested scopes, and, finally,
the scopes created by ``block()`` command are removed.

``SCOPE_FOR``
  Specify which scopes must be created.

  ``POLICIES``
    Create a new policy scope. This is equivalent to
    :command:`cmake_policy(PUSH)`.

  ``VARIABLES``
    Create a new variable scope.

  If ``SCOPE_FOR`` is not specified, this is equivalent to:

  .. code-block:: cmake

    block(SCOPE_FOR VARIABLES POLICIES)

``PROPAGATE``
  When a variable scope is created by :command:`block` command, this option
  set or unset the specified variables in the parent scope. This is equivalent
  to :command:`set(PARENT_SCOPE)` or :command:`unset(PARENT_SCOPE)` commands.

  .. code-block:: cmake

    set(VAR1 "INIT1")
    set(VAR2 "INIT2")

    block(PROPAGATE VAR1 VAR2)
      set(VAR1 "VALUE1")
      unset(VAR2)
    endblock()

    # here, VAR1 holds value VALUE1 and VAR2 is unset

  This option is only allowed when a variable scope is created. An error will
  be raised in the other cases.

When the ``block`` is local to a :command:`foreach` or :command:`while`
command, the commands :command:`break` and :command:`continue` can be used
inside this block.

.. code-block:: cmake

  while(TRUE)
    block()
       ...
       # the break() command will terminate the while() command
       break()
    endblock()
  endwhile()


See Also
^^^^^^^^

  * :command:`endblock`
  * :command:`cmake_policy`
