FindPython-DEBUG
----------------

* The :module:`FindPython`, :module:`FindPython2` and :module:`FindPython3`
  modules offer, on ``Windows`` platform, a better support of the ``Python``
  debug version:

  * new variables:

    * ``Python_EXECUTABLE_DEBUG``
    * ``Python_INTERPRETER``
    * ``Python_DEBUG_POSTFIX``

  * new targets:

    * ``Python::InterpreterDebug``
    * ``Python::InterpreterMultiConfig``

  And the ``python_add_library()`` command manage the :prop_tgt:`DEBUG_POSTFIX`
  target property based on the value of the ``Python_DEBUG_POSTFIX`` variable.
