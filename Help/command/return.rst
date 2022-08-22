return
------

Return from a file, directory or function.

.. code-block:: cmake

  return([PROPAGATE <var-name>...])

Returns from a file, directory or function.  When this command is
encountered in an included file (via :command:`include` or
:command:`find_package`), it causes processing of the current file to stop
and control is returned to the including file.  If it is encountered in a
file which is not included by another file, e.g.  a ``CMakeLists.txt``,
deferred calls scheduled by :command:`cmake_language(DEFER)` are invoked and
control is returned to the parent directory if there is one.  If return is
called in a function, control is returned to the caller of the function.

``PROPAGATE``
  .. versionadded:: 3.25

  This option set or unset the specified variables in the parent directory or
  function caller scope. This is equivalent to :command:`set(PARENT_SCOPE)` or
  :command:`unset(PARENT_SCOPE)` commands.

  The option ``PROPAGATE`` can be very useful in conjunction with the
  :command:`block` command because the :command:`return` will cross over
  various scopes created by the :command:`block` commands.

  .. code-block:: cmake

    function(MULTI_SCOPES RESULT_VARIABLE)
      block(SCOPE_FOR VARIABLES)
        # here set(PARENT_SCOPE) is not usable because it will not set the
        # variable in the caller scope but in the parent scope of the block()
        set(${RESULT_VARIABLE} "new-value")
        return(PROPAGATE ${RESULT_VARIABLE})
      endblock()
    endfunction()

    set(MY_VAR "initial-value")
    multi_scopes(MY_VAR)
    # here MY_VAR will holds "new-value"

Policy :policy:`CMP0140` controls the behavior regarding the arguments of the
command.

Note that a :command:`macro <macro>`, unlike a :command:`function <function>`,
is expanded in place and therefore cannot handle ``return()``.

See Also
^^^^^^^^

  * :command:`block`
