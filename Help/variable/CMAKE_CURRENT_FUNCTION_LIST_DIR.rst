CMAKE_CURRENT_FUNCTION_LIST_DIR
-------------------------------

When executing code inside a :command:`function`, this variable
contains the full directory of the listfile defining the current function.

It is quite common practice in CMake that modules use some additional files
(e.g., templates to render).  And the code typically did the following:

.. code-block:: cmake
    :caption: Bad

    set(_THIS_MODULE_BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")

    function(foo)
      configure_file(
        "${_THIS_MODULE_BASE_DIR}/some.template.in"
        some.output
      )
    endfunction()

Using this variable inside a function eliminates the neccessity of the
additional one with "global" scope:

.. code-block:: cmake
    :caption: Good

    function(foo)
      configure_file(
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/some.template.in"
        some.output
      )
    endfunction()
